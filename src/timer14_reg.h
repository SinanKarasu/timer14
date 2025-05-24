/* @(#)timer14_reg.h	" */




/*
 * default values for the limit structure
 */
#define TIMER14_DEFAULT_LOWER	(u_long)0x00000000	/* any address */
#define TIMER14_DEFAULT_UPPER	(u_long)0xFFFFFFFF	/* any address */

/*
 * bit fields for Timer14.flags
 */
#define TIMER14_DEV_OPEN	0x00000001
#define TIMER14_DEV_BUSY	0x00000002

/*
 * bit fields for Timer14.attach_flags
 */
#define SOFT_STATE_ALLOCATED	0x01
#define INTERRUPT_ADDED		0x02
#define MUTEX_ADDED		0x04
#define CV_ADDED		0x08
#define MINOR_NODE_CREATED	0x10
#define SOFTINT_ADDED		0x40


/*
 * other stuff
 */
#ifndef	TRUE
#define	TRUE	1
#define	FALSE	0
#endif	TRUE

#ifndef	OK
#define	OK	0
#endif	OK

/*-----------------------------*/

#define MINPERIOD		25      /* Min. interrupt-period (usec) */

#ifdef sun4u

#define	TIMER14_NAME	"timer14"	/* name string  */
#define TIMER14_NODE    "ddi_pseudo"

#define	CTR_USEC_MASK		0xFFFFFFFF	/* counter/limit mask */
#define	CTR_USEC_SHIFT		0		/* counter/limit shift */

volatile uint interrupt_count;
volatile uint interrupts_missed;

#define NUMBER_OF_CPUS NCLKS

#endif

#define USECS(time) (((time) & CTR_USEC_MASK) >> CTR_USEC_SHIFT)

#ifdef sun4m
#define	TIMER14_NAME	"timer14"	/* name string  */
#define TIMER14_NODE    "SUNW,Sun 4_75"
#define counter  v_counter_addr[0]
#define INTR14_ON(n)	v_level10clk_addr->config &=~(1<<n)
#define INTR14_OFF(n)	v_level10clk_addr->config |=(1<<n)
#define INTR14_OFF_ALL(m) v_level10clk_addr->config |=m
#define	VALID_CPU(n)	((cpu[n] != NULL) && (cpu[n]->cpu_flags & CPU_RUNNING))
#define timer14_setlimit(n,value) v_counter_addr[n]->timer_msw=value
#define timer14_getlimit(n,value) *value=v_counter_addr[n]->timer_msw
#define timer14_getcount(n,value) *value=v_counter_addr[n]->timer_lsw
#define NUMBER_OF_CPUS 4
#endif

#ifdef sun4c
#define	TIMER14_NAME	"timer14"	/* name string  */
#define TIMER14_NODE    "SUNW,Sun 4_75"
extern volatile struct counterregs *counter = COUNTER;
#define timer14_setlimit(n,value) counter->limit14=value
#define timer14_getlimit(n,value) *value=counter->limit14
#define timer14_getcount(n,value) *value=counter->counter14
#define INTR14_ON	set_clk_mode(IR_ENA_CLK14, 0)
#define INTR14_OFF	set_clk_mode(0, IR_ENA_CLK14)
#endif

#ifdef sun4d
#define	TIMER14_NAME	"timer14"	/* name string  */
#define TIMER14_NODE    "ddi_pseudo"
extern void		st_a_2f(u_int value, u_int *addr);
extern u_int	ld_a_2f(u_int *addr);
#ifndef BUS_SHIFT
#define BUS_SHIFT 8
#endif

extern	struct cpu cpu0;
#define CPUid2BWcsrid(cid) ((cid|CSR_CPU_PREFIX)<<CPU_2_CSR_SHIFT)
#define timer14_limitreg(cid,bus) \
			(u_int *)(CPUid2BWcsrid(cid)|(bus<<BUS_SHIFT)|OFF_BW_PROFILE_TIMER_LIMIT)
#define timer14_setlimit_cid(cid,bus,value) \
		st_a_2f(value,timer14_limitreg(cid,bus))
#define timer14_setlimit(value) timer14_setlimit_cid(cpu0.cpu_id,0,value)
#define timer14_getlimit_cid(cid,bus,value) \
		*value = ld_a_2f(timer14_limitreg(cid,bus))
#define timer14_getlimit(dest) timer14_getlimit_cid(cpu0.cpu_id,0,dest)

#define timer14_countreg(cid,bus) \
			(u_int *)(CPUid2BWcsrid(cid)|(bus<<BUS_SHIFT)|OFF_BW_PROFILE_TIMER_COUNTER)
#define timer14_getcount_cid(cid,bus,value) \
		*value = ld_a_2f(timer14_countreg(cid,bus))
#define timer14_getcount(dest) timer14_getcount_cid(cpu0.cpu_id,0,dest)

#define INTR14_ON  /* not used in sun4d. just program it */
#define INTR14_OFF timer14_setlimit(0)
#endif




typedef struct timer14 {				/* per-unit structure */
	dev_info_t		*dip;
	kmutex_t		mutex;
	kcondvar_t		cv;
	ddi_iblock_cookie_t	iblock_cookie;	/* for mutexes */
	ddi_dma_handle_t	handle;
	struct buf		*bp;
	volatile	u_int			attach_flags;
	volatile	u_int			flags;
	volatile	u_int			intr_count;
	volatile	u_int			period[NUMBER_OF_CPUS];
	volatile	u_int			counter_limit[NUMBER_OF_CPUS];
	volatile	u_int			counter_triggered[NUMBER_OF_CPUS];
	volatile	u_int			interrupt_count[NUMBER_OF_CPUS];
	volatile	u_int			interrupts_missed[NUMBER_OF_CPUS];
	volatile	u_int			Cpus_Active;
	ddi_softintr_t		softintr_id;
} Timer14;

