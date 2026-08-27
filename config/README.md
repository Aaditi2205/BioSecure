# Provisioning

The example intentionally contains no credentials. Provision Wi-Fi credentials and
API tokens into encrypted NVS using a controlled manufacturing/setup flow. Do not
commit populated configuration. Production builds should enable ESP32 secure boot,
flash encryption and NVS encryption after validating the irreversible provisioning
procedure for the chosen module.
