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
