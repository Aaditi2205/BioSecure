# Presentation-attack limitations

This project has no evidence that an R307 provides robust liveness or standardized
presentation-attack detection (PAD), so it makes no such claim. Optical fingerprint
sensors can be attacked with replicas or latent-print techniques. Retry delay,
failed-attempt counting, lockout and audit logging reduce automated abuse but are not
PAD and do not establish that a living, authorized person is present.

`IPresentationAttackDetector` is a fail-closed policy port: a future documented PAD
module may return PASS/FAIL/UNAVAILABLE before matching is accepted. Deployments that
require stronger assurance should combine independently evaluated PAD, a second
factor, supervised enrollment, tamper resistance and an explicit risk assessment.
