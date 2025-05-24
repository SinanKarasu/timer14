# HISTORY.md – Reflections on Timer14 Development

## Real-Time Timing Performance

During early development and testing of Timer14, a series of empirical latency measurements were taken across typical Sun hardware available to engineers at the time.

- **SPARCstation LX** (ubiquitous desktop system):
  - Latency: ~260 microseconds
- **SPARCstation 20** (lab-grade multiprocessing box):
  - Latency: <40 microseconds
- **Interrupt response** (hardware to ISR entry): ~6 microseconds

These were measured without specialized instrumentation, and demonstrated Timer14’s ability to reliably synchronize real-time workloads across platforms where the default Solaris timer resolution (~10ms) was wholly inadequate for simulation purposes.

## A Curious Windows 2 Experiment

Prior to Timer14, the developer had been working on a hardware interface project that involved emulating computer responses to test a device under development. For fun (and out of professional curiosity), he attempted to perform similar timing work under Windows 2.

- Interrupt response under Windows 2 was observed at approximately 140 microseconds.
- In a moment of inspiration (or perhaps hubris), two cards were installed to test interleaved interrupt handling.

The result?

> “I belatedly realized you can't serve two interleaved interrupts at the same time. I never mentioned it to anybody lest they think I'm an idiot.”

The real story here is the humility and insight: the test quietly revealed a fundamental limitation in how early non-reentrant PC interrupt handling worked. No harm, no crash — just learning, integrated silently into later, stronger designs.

## Conclusion

Timer14 wasn’t built on first guesses — it was built on a deep curiosity about timing, interrupt delivery, system architecture, and how real systems behave when tested. That mindset, not just the code, is what made it so reliable.

And it never needed to be perfect. It just needed to work — 1.44 million ticks per night, two misses, and the respect of everyone who ever depended on it.

—  
**Sinan Karasu**  
Boeing  
Early 1990s


---

### ☠️ The SPARCstation LX Watchdog Reset

While developing and testing `timer14`, a particularly educational moment occurred during cross-platform validation.

On the **dual-CPU SPARCstation 20**, the timer system worked as expected — each CPU independently received and processed timer14 interrupts.

However, on the **single-CPU SPARCstation LX**, running the same logic triggered a **Watchdog Timer Reset**.

The cause: per-CPU interrupt logic or excessive interrupt load blocked the kernel from responding to system-level heartbeats. The hardware watchdog interpreted this as a lockup and rebooted the system to preserve integrity.

> “That was educational.”

This served as a powerful reminder:
- Real-time kernel logic must gracefully scale **down** as well as up
- The Solaris watchdog is a real, hardware-enforced failsafe — and not just a logging feature
- On slower or uniprocessor systems, interrupt frequency and service time must be tuned carefully

