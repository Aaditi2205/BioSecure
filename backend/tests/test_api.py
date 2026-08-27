import importlib
from fastapi.testclient import TestClient

EVENT = {"event_uuid":"12345678-1234-4123-a123-123456789abc","sequence":1,"device_id":"SIM-001","sensor_template_id":42,"application_user_id":"demo","timestamp_ms":1700000000000,"authentication_result":"granted","sync_state":"pending","time_quality":"synchronized","firmware_version":"0.1.0"}

def client(tmp_path, monkeypatch):
    monkeypatch.setenv("BIOSECURE_DB_PATH", str(tmp_path / "test.db"))
    import app.main as main
    importlib.reload(main)
    return TestClient(main.app)

def test_idempotent_create_and_list(tmp_path, monkeypatch):
    c=client(tmp_path,monkeypatch)
    headers={"Idempotency-Key":EVENT["event_uuid"]}
    first=c.post("/api/v1/attendance",json=EVENT,headers=headers)
    repeat=c.post("/api/v1/attendance",json=EVENT,headers=headers)
    assert first.status_code==201 and repeat.status_code==201
    assert first.json()["event_uuid"]==repeat.json()["event_uuid"]
    assert len(c.get("/api/v1/attendance").json())==1

def test_rejects_biometric_and_bad_key(tmp_path, monkeypatch):
    c=client(tmp_path,monkeypatch)
    bad=dict(EVENT,fingerprint_image="forbidden")
    assert c.post("/api/v1/attendance",json=bad,headers={"Idempotency-Key":EVENT["event_uuid"]}).status_code==422
    assert c.post("/api/v1/attendance",json=EVENT,headers={"Idempotency-Key":"wrong"}).status_code==400
