# Timer14 – Kernel-Level Real-Time Timer for Solaris

**Author:** Sinan Karasu  
**Originally developed at:** Boeing  
**Date:** Early 1990s

Timer14 is a Solaris kernel module providing sub-10ms high-resolution timers for real-time applications. Originally designed for Boeing’s 777 simulation systems, it enabled precise 25ms execution frames — a critical requirement unmet by Solaris’s default 10ms system clock of the era.

## 🛠 Features

- Direct use of interrupt 14 to establish kernel-level timing
- SPARC assembly integration for register-level timing control
- Provided core infrastructure for aerospace simulation systems
- Later referenced in Sun internal research and academic RTOS development

## 🧾 Historical Significance

Timer14 was acknowledged in:
- Marc Hamilton’s 1995 Sun Microsystems technical report:
> “This work would have never been possible if not for the original timer14 device driver written by Sinan Karasu of Boeing.”
- RTCOS research by University of Maryland (1996)
- Deployed in Boeing, TRW, and Northrop Grumman labs

## 📁 Directory Layout

- `src/` – Kernel module source (`.c`, `.h`, SPARC `.s`, Makefile)
- `install/` – User-space installer utility (for legacy Solaris environments)

## 📜 License

Released into the public domain or equivalent.  
No warranty expressed or implied.  
Preserved here for historical and educational purposes.

## 🙏 Acknowledgements

Special thanks to Linda Knapp (Sun Microsystems, DSEM Seattle)  
for bridging Sun and Boeing teams during this work’s development.

---

This is a preserved piece of UNIX history.  
If you ever used Solaris in real-time contexts, this is part of your lineage.
