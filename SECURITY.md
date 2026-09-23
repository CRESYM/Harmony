# Security Policy

Harmony relies on and interfaces with external libraries and third-party dependencies, including experimental software that is not validated, tested or maintained by the Harmony team. As a result, calling functions or otherwise interacting with these dependencies may introduce unintended or unsafe behavior into Harmony, including behavior that has not been reviewed or anticipated by the Harmony team.

Because Harmony is designed to run with the permissions of the invoking user account:

* Avoid running or compiling Harmony as a superuser or administrator.
* Do not execute Harmony using `sudo`, `su`, or from an elevated administrative terminal. Always run Harmony within an unprivileged user environment.

### Reporting Standard Bugs and Crashes
If you encounter a crash, unexpected behavior, unexpected file modification, or other issue during normal use:
* Please check your input parameters and environment configuration.
* If the issue persists and you believe it is a bug in Harmony's core logic, report it via a [GitHub Issue](https://github.com/CRESYM/Harmony/issues).

### Reporting Security Concerns
If you discover a security vulnerability:
1. Do **not** open a public GitHub issue.
2. [Contact the team](https://cresym.github.io/Harmony-Doc/pages/page_layout.html?file=contact) and send a detailed report including clear steps to reproduce the issue and a minimal proof of concept.