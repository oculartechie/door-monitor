# door-monitor
Arduino/Particle Photon sketch to detect when someone rings the doorbell

Before running sketch, you will need to define a few macros

SMTP_SERVER      - SMTP email server
SMTP_USER_BASE64 - SMTP email username (optional if not required to use email server)
SMTP_PASS_BASE64 - SMTP email password (optional if not required to use email server)

SMTP_FROM_EMAIL  - Sender
SMTP_TO_EMAIL    - Receiver
SMTP_SUBJECT     - Email subject
SMTP_BODY        - Email body (optional)
