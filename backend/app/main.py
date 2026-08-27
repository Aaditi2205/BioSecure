import json
import os
import sqlite3
from contextlib import contextmanager
from enum import Enum
from pathlib import Path
from typing import Iterator

from fastapi import FastAPI, Header, HTTPException, Query
from pydantic import BaseModel, ConfigDict, Field, model_validator

DB_PATH = Path(os.getenv("BIOSECURE_DB_PATH", "biosecure.db"))
FORBIDDEN = {"fingerprint", "image", "raw_sample", "features", "template", "template_data", "biometric"}

class TimeQuality(str, Enum):
    synchronized = "synchronized"
    estimated = "estimated"
    unknown = "unknown"

class AttendanceIn(BaseModel):
    model_config = ConfigDict(extra="forbid")
    event_uuid: str = Field(min_length=36, max_length=36)
    sequence: int = Field(ge=1)
    device_id: str = Field(min_length=1, max_length=128)
    sensor_template_id: int = Field(ge=0, le=65535)
    application_user_id: str = Field(default="", max_length=128)
    timestamp_ms: int
    authentication_result: str = Field(pattern="^(granted|denied)$")
    sync_state: str = Field(pattern="^pending$")
    time_quality: TimeQuality
    firmware_version: str = Field(min_length=1, max_length=64)

    @model_validator(mode="before")
    @classmethod
    def block_biometric_fields(cls, value):
        if isinstance(value, dict) and any(k.lower() in FORBIDDEN for k in value):
            raise ValueError("biometric material is forbidden")
        return value

class AttendanceOut(AttendanceIn):
    received_at: str

@contextmanager
def db() -> Iterator[sqlite3.Connection]:
    connection = sqlite3.connect(DB_PATH)
    connection.row_factory = sqlite3.Row
    try:
        connection.execute("CREATE TABLE IF NOT EXISTS attendance (event_uuid TEXT PRIMARY KEY, idempotency_key TEXT UNIQUE NOT NULL, sequence INTEGER NOT NULL, device_id TEXT NOT NULL, body TEXT NOT NULL, received_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP, UNIQUE(device_id, sequence))")
        yield connection
        connection.commit()
    finally:
        connection.close()

app = FastAPI(title="BioSecure metadata API", version="1.0.0")

@app.post("/api/v1/attendance", response_model=AttendanceOut, status_code=201)
def create_attendance(event: AttendanceIn, idempotency_key: str = Header(alias="Idempotency-Key")):
    if idempotency_key != event.event_uuid:
        raise HTTPException(400, "Idempotency-Key must equal event_uuid")
    body = event.model_dump_json()
    with db() as connection:
        existing = connection.execute("SELECT body, received_at FROM attendance WHERE idempotency_key=?", (idempotency_key,)).fetchone()
        if existing:
            original = json.loads(existing["body"])
            original["received_at"] = existing["received_at"]
            return original
        try:
            connection.execute("INSERT INTO attendance(event_uuid,idempotency_key,sequence,device_id,body) VALUES(?,?,?,?,?)", (event.event_uuid,idempotency_key,event.sequence,event.device_id,body))
        except sqlite3.IntegrityError as exc:
            raise HTTPException(409, "sequence conflict") from exc
        received = connection.execute("SELECT received_at FROM attendance WHERE event_uuid=?", (event.event_uuid,)).fetchone()[0]
    result = event.model_dump(mode="json")
    result["received_at"] = received
    return result

@app.get("/api/v1/attendance", response_model=list[AttendanceOut])
def list_attendance(limit: int = Query(100, ge=1, le=1000), offset: int = Query(0, ge=0)):
    with db() as connection:
        rows = connection.execute("SELECT body, received_at FROM attendance ORDER BY device_id, sequence LIMIT ? OFFSET ?", (limit, offset)).fetchall()
    return [dict(json.loads(row["body"]), received_at=row["received_at"]) for row in rows]

@app.get("/healthz")
def health(): return {"status": "ok"}
