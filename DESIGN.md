# DESIGN.md – Timer14 Architecture and Real-Time Behavior

## Overview

`timer14` is a Solaris kernel driver providing high-resolution real-time scheduling by binding to **interrupt level 14**. It was designed for flight simulation systems that required **precise 25ms execution frames**, surpassing the standard Solaris 10ms timer granularity.

## Key Behavior

Applications using `timer14` follow this sequence:

1. Perform computation or simulation logic
2. Call `ioctl(fd, TIMER14_WAIT_TICK)` to:
   - Wait until the **next precise timer interrupt**
   - OR immediately return if the tick was missed (overdue)

This model eliminates the need for:
- Busy-wait loops
- Sleep-based polling
- External real-time patches

## Timing Model

```
+----------+           +----------+           +----------+
|  Logic   | ioctl() → | Timer 14 | → Return  |  Logic   |
+----------+           +----------+           +----------+
   Frame 1                Wait                    Frame 2
```

Each `ioctl()` blocks until the next frame tick, maintaining perfect alignment.

## Performance

> “One of the engineers didn’t believe me and ran it overnight.
> There were only two tick misses. He said that was amazing.” — Sinan Karasu

- Frame rate: 25ms = 40Hz
- Overnight run = ~1.44 million ticks
- **Tick accuracy > 99.9998%**

## ioctl Interface (from `timer14_io.h`)

```c
/* @(#)timer14_io.h	" */


/*
 * commands for timer14_ioctl
 */
#define	TIMER14_GET_CSR		_IOR('b', 1, Timer14_csr)

/*
 * register definitions
 */
typedef struct timer14_status{
	u_int	counter_reg;
	u_int	limit_reg;
	u_int	period;
	u_int	intr_count;
	u_int	interrupts_missed;
} Timer14_status;


#define TIMER14_VERSION 1002

enum param {TIMER14_ENQUIRE,
			TIMER14_START,
			TIMER14_INIT,
			TIMER14_WAIT,
			TIMER14_STOP,
			TIMER14_WAIT_SPECIFIC,
			TIMER14_STOP_SPECIFIC,
			TIMER14_START_SPECIFIC,
			TIMER14_ENQUIRE_SPECIFIC
			};

```

## Design Notes

- Uses **SPARC assembly** to manage alternate loads/stores
- Avoids scheduler modifications entirely
- Leverages interrupt 14 for consistent timing alignment
- Originally tested on SPARCstation 1, later deployed on Sun 670MP and SS10

## Real-Time Goals Met

- Accurate frame alignment (±<0.1ms)
- No drift, no race conditions
- Works with Fortran applications and large simulation codebases without rewriting

## Author

**Sinan Karasu**  
Developed at Boeing for 777 simulation systems  
Collaborated with Brian One at SunSoft to bind to interrupt 14

---

This file documents the architectural brilliance behind a user-space controllable, interrupt-aligned, high-resolution timer — done entirely within the Solaris driver framework.


---

### 🧠 Per-CPU Interrupt Design

Timer14 installs an interrupt handler for **each CPU** on the system, using interrupt level 14 independently. This per-CPU binding allows:

- Parallel execution of simulation frames on multiprocessor SPARC systems
- Accurate frame timing even in CPU-intensive workloads
- Minimal contention and drift across cores

This approach was used successfully on systems like the **Sun 670MP** and **SPARCstation 10**, enabling real-time flight simulation across multiple CPUs.

---

### 🙌 A Note of Thanks

Timer14’s completion was supported by **Brian One at SunSoft**, who provided essential insight on interrupt attachment.  

**Linda Knapp**, DSEM at Sun Microsystems (Seattle), was instrumental in ensuring Sun engineering resources aligned with Boeing's simulation needs. Her support helped the project reach the right engineers at the right time.

---
