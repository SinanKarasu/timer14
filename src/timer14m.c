/**********************************************************************/
#include <sys/types.h>
#include <sys/param.h>
#include <sys/errno.h>
#include <sys/file.h>
#include <sys/conf.h>
#include <sys/uio.h>
#include <sys/map.h>
#include <sys/debug.h>
#include <sys/modctl.h>
#include <sys/kmem.h>
#include <sys/cmn_err.h>
#include <sys/open.h>
#include <sys/stat.h>

#if defined (sun4m) || defined (sun4c) 
#include <sys/intreg.h>
#endif

#ifdef sun4d
#include <sys/physaddr.h>
#include <sys/vtrace.h>
#endif

#include <sys/clock.h>
#include <sys/ddi.h>
#include <sys/sunddi.h>
#include <sys/avintr.h>

#include <fcntl.h>

#include "timer14_io.h"
#include "timer14_reg.h"

/*#define TIMER14_DEBUG*/

static	void *state_head;	/* opaque handle top of state structs */

/*
 * These are the entry points into our driver that are called when the
 * driver is loaded, during a system call, or in response to an interrupt.
 */
static	int	timer14_getinfo(dev_info_t *dip, ddi_info_cmd_t infocmd, void *arg, 
		    void **result);
static	int	timer14_identify(dev_info_t *dip);
static  int	timer14_probe(dev_info_t*);

static	int	timer14_attach(dev_info_t *dip, ddi_attach_cmd_t cmd);
static	int	timer14_open(dev_t *dev, int openflags, int otyp, cred_t *credp);
static	int	timer14_close(dev_t dev, int openflags, int otyp, cred_t *credp);
static	int	timer14_ioctl(dev_t dev, int cmd, int arg, int flag, cred_t *credp,
		    int *rvalp);
static	u_int	timer14_intr(caddr_t arg);
static	int	timer14_detach(dev_info_t *dip, ddi_detach_cmd_t cmd);

static u_int 	timer14_softintr(caddr_t arg);


/*
 * When our driver is loaded or unloaded, the system calls our _init or
 * _fini routine with a modlinkage structure.  The modlinkage structure
 * contains:
 *
 *	modlinkage->
 *		modldrv->
 *			dev_ops->
 * 				cb_ops
 *
 * cb_ops contains the normal driver entry points and is roughly equivalent
 * to the cdevsw & bdevsw structures in previous releases.
 *
 * dev_ops contains, in addition to the pointer to cb_ops, the routines
 * that support loading and unloading our driver.
 */
static struct cb_ops	timer14_cb_ops = {
	timer14_open,
	timer14_close,
	nodev,		/* not a block driver, strategy not an entry point */
	nodev,		/* no print routine */
	nodev,		/* no dump routine */
	nodev,		/* no read routine */
	nodev,		/* no write routine */
	timer14_ioctl,
	nodev,		/* no devmap routine */
	nodev,		/* no mmap routine */
	nodev,		/* no segmap routine */
	nochpoll,	/* no chpoll routine */
	ddi_prop_op,
	0,		/* not a STREAMS driver, no cb_str routine */
	D_NEW | D_MP,	/* safe for multi-thread/multi-processor */
};

static struct dev_ops timer14_ops = {
	DEVO_REV,		/* DEVO_REV indicated by manual	*/
	0,			/* device reference count	*/
	timer14_getinfo,
	timer14_identify,	/* timer14_identify */
	timer14_probe,		/* device probe for non-self-id */
	timer14_attach,
	timer14_detach,
	nodev,			/* device reset routine		*/
	&timer14_cb_ops,
	(struct bus_ops *)NULL,	/* bus operations		*/
};

extern	struct	mod_ops mod_driverops;
static	struct modldrv modldrv = {
	&mod_driverops,
	"timer14",
	&timer14_ops,
};

static	struct modlinkage modlinkage = {
	MODREV_1,		/* MODREV_1 indicated by manual */
	(void *)&modldrv,
	NULL,			/* termination of list of linkage structures */
};

volatile static int Timer14_SRQ=0;


/*
 * _init, _info, and _fini support loading and unloading the driver.
 */
int
_init(void)
{
	register int	error;

	if ((error = ddi_soft_state_init(&state_head, sizeof (Timer14), 1)) !=0){
		cmn_err(CE_NOTE,"ddi_soft_state_init %d \n",error);
		return(error);
	}

	if ((error = mod_install(&modlinkage)) != 0)
		ddi_soft_state_fini(&state_head);
#ifdef TIMER14_DEBUG
	cmn_err(CE_NOTE,"mod_install %d \n",error);
#endif
	return (error);
}

int
_info(struct modinfo *modinfop)
{	int error;
	error = (mod_info(&modlinkage, modinfop));
#ifdef TIMER14_DEBUG
	cmn_err(CE_NOTE," info return is %d \n",error);
#endif
	return(error);
}

int
_fini(void)
{
	int status;

	if ((status = mod_remove(&modlinkage)) != 0)
		return (status);
	
	ddi_soft_state_fini(&state_head);

	return(status);

}

/*
 * This is a pretty generic getinfo routine as described in the manual.
 */
/*ARGSUSED*/
static int
timer14_getinfo(dev_info_t *dip, ddi_info_cmd_t infocmd, void *arg, void **result)
{
	register int 	error;
	register Timer14	*timer14_p;

	switch (infocmd) {
	case DDI_INFO_DEVT2DEVINFO:
		timer14_p = (Timer14 *)ddi_get_soft_state(state_head, ((dev_t)arg));
		if (timer14_p == NULL) {
			*result = NULL;
			error = DDI_FAILURE;
		} else {
			/*
			 * don't need to use a MUTEX even though we are
			 * accessing our instance structure; dma_p->dip
			 * never changes.
			 */


			*result = timer14_p->dip;
			error = DDI_SUCCESS;
		}
		break;
	case DDI_INFO_DEVT2INSTANCE:
		*result = (void *)getminor((dev_t)arg);
		error = DDI_SUCCESS;
		break;
	default:

		*result = NULL;
		error = DDI_FAILURE;
	}

	return (error);
}

static int
timer14_identify(dev_info_t *dip)
{
	char *dev_name;

	dev_name = ddi_get_name(dip);

	if (strcmp(dev_name, TIMER14_NAME) == 0) {
#ifdef TIMER14_DEBUG
	cmn_err(CE_NOTE,"identified \n");
#endif

		return (DDI_IDENTIFIED);
	} else {
#ifdef TIMER14_DEBUG
	cmn_err(CE_NOTE,"not identified \n");
#endif
		return (DDI_NOT_IDENTIFIED);
	}
}

static int
timer14_probe(dev_info_t *dip)
{

	return (DDI_PROBE_SUCCESS);
}

static int
timer14_attach(dev_info_t *dip, ddi_attach_cmd_t cmd)
{
	int			instance;
	register Timer14		*timer14_p;
	int			result;
	int			error;
	ddi_iblock_cookie_t icl;
	ddi_idevice_cookie_t idc;

	if (cmd != DDI_ATTACH)
		goto bailout;


	instance = ddi_get_instance(dip);


	if (ddi_soft_state_zalloc(state_head, instance) != DDI_SUCCESS)
		goto bailout;

	timer14_p = (Timer14 *)ddi_get_soft_state(state_head, instance);
	timer14_p->attach_flags |= SOFT_STATE_ALLOCATED;
	timer14_p->dip = dip;
	if(!ddi_intr_hilevel(dip,0)){
		cmn_err(CE_WARN," NOT A high level interrupt \n");
		goto bailout;
	}
 
	
	/* need to use a soft interrupt, since we must call cv_signal()
	 * and we can only do this out of a thread context, running at 
	 * less than level 10
	 */

	if (ddi_add_softintr(dip, DDI_SOFTINT_HIGH,
	    &timer14_p->softintr_id, &icl, &idc, timer14_softintr,
			     (caddr_t)dip) != DDI_SUCCESS) { 
			cmn_err(CE_WARN, "timer14: can't add soft interrupt\n");
			goto bailout;
       	}

	timer14_p->attach_flags |= SOFTINT_ADDED;

	if (ddi_add_intr(dip, 0, &timer14_p->iblock_cookie,
			 /*(ddi_iblock_cookie_t *)NULL,  */
			 (ddi_idevice_cookie_t *) NULL, timer14_intr,
			 (caddr_t) dip) != DDI_SUCCESS) {
	    cmn_err(CE_WARN,"timer14: can't claim interrupt!\n");
	    goto bailout;
	} 
	else {
	    timer14_p->attach_flags |= INTERRUPT_ADDED;
	}


	timer14_p = (Timer14 *)ddi_get_soft_state(state_head, instance);
	mutex_init(&timer14_p->mutex, "timer14 mutex",
		MUTEX_DRIVER, (void *)&timer14_p->iblock_cookie);
		timer14_p->attach_flags |= MUTEX_ADDED;

	cv_init(&timer14_p->cv,"timer14 cv",
		CV_DRIVER, (void *)&timer14_p->iblock_cookie);
		timer14_p->attach_flags |= CV_ADDED;



	/*
	 * ddi_create_minor_node creates an entry in an internal kernel
	 * table; the actual entry in the file system is created by
	 * drvconfig(1) when you run add_drv(1);
	 */
	if (ddi_create_minor_node(dip, ddi_get_name(dip), S_IFCHR, instance,
		TIMER14_NODE,0) == DDI_FAILURE) {
		cmn_err(CE_WARN,"ddi_minor_create failed");
		ddi_remove_minor_node(dip, NULL);
		goto bailout;
	} else {
		timer14_p->attach_flags |= MINOR_NODE_CREATED;
	}

	Timer14_SRQ=0;
	timer14_p->Cpus_Active=0;  
	return (DDI_SUCCESS);

/*
 * should probably change this to just call timer14_detach
 */
bailout:

	if (timer14_p->attach_flags & SOFTINT_ADDED)
	    ddi_remove_softintr(timer14_p->softintr_id);

	if (timer14_p->attach_flags & INTERRUPT_ADDED)
		ddi_remove_intr(timer14_p->dip, 0, timer14_p->iblock_cookie);
	if (timer14_p->attach_flags & SOFT_STATE_ALLOCATED)
		ddi_soft_state_free(state_head, instance);
	if (timer14_p->attach_flags & MUTEX_ADDED)
		mutex_destroy(&timer14_p->mutex);
	if (timer14_p->attach_flags & CV_ADDED)
		cv_destroy(&timer14_p->cv);
	if (timer14_p->attach_flags & MINOR_NODE_CREATED)
		ddi_remove_minor_node(dip, NULL);

	return (DDI_FAILURE);
}

/*
 * When our driver is unloaded, timer14_detach cleans up and frees the resources
 * we allocated in timer14_attach.
 */
static int
timer14_detach(dev_info_t *dip, ddi_detach_cmd_t cmd)
{
	register Timer14	*timer14_p;	/* will point to this */
	int		instance;

	if (cmd != DDI_DETACH)
		return (DDI_FAILURE);

	instance = ddi_get_instance(dip);
	timer14_p = (Timer14 *)ddi_get_soft_state(state_head, instance);

	/* Remove the minor node created in attach */
	ddi_remove_minor_node(dip, NULL);


	/*
	 * deallocate resources from attach
	 */
	if (timer14_p->attach_flags & SOFTINT_ADDED)
	    ddi_remove_softintr(timer14_p->softintr_id);
	if (timer14_p->attach_flags & INTERRUPT_ADDED)
		ddi_remove_intr(timer14_p->dip, 0, timer14_p->iblock_cookie);
	if (timer14_p->attach_flags & MUTEX_ADDED)
		mutex_destroy(&timer14_p->mutex);
	if (timer14_p->attach_flags & CV_ADDED)
		cv_destroy(&timer14_p->cv);
	if (timer14_p->attach_flags & MINOR_NODE_CREATED)
		ddi_remove_minor_node(dip, NULL);
	if (timer14_p->attach_flags & SOFT_STATE_ALLOCATED)
		ddi_soft_state_free(state_head, instance);

	return (DDI_SUCCESS);
}

/*
 * timer14_open is called in response to the open(2) system call
 */
/*ARGSUSED*/
static	int
timer14_open(dev_t *dev, int openflags, int otyp, cred_t *credp)
{
	int		retval = 0;
	register Timer14	*timer14_p;

	int dummy;

	timer14_p = (Timer14 *)ddi_get_soft_state(state_head, getminor(*dev));

	dummy=getminor(*dev);


	/*
	 * Verify instance structure
	 */

	if (timer14_p == NULL) return (ENXIO);


	/* lock control status register and exclusive open flag */
	mutex_enter(&timer14_p->mutex);			/* start MUTEX */

	
	if( openflags & FWRITE){/* Only allow a single open O_RDWR */
		if (timer14_p->flags & TIMER14_DEV_OPEN) {
			retval = EBUSY;
		} else {
			/* mark device as open */
			timer14_p->flags |= TIMER14_DEV_OPEN;
		}
	}


	mutex_exit(&timer14_p->mutex);			/* end MUTEX */

	return (retval);
}

/*ARGSUSED*/
static	int
timer14_close(dev_t dev, int openflags, int otyp, cred_t *credp)
{
	register Timer14	*timer14_p;

	timer14_p = (Timer14 *)ddi_get_soft_state(state_head, getminor(dev));

	/*
	 * lock control status register and exclusive open flag
	 */
	mutex_enter(&timer14_p->mutex);			/* start MUTEX */

	/*
	 * let the next person open the device
	 */

	timer14_p->flags &= ~TIMER14_DEV_OPEN;


	/*
	 * shut down the devices
	*/

 	INTR14_OFF_ALL(timer14_p->Cpus_Active);  /* Interrupts off */
	timer14_p->Cpus_Active=0;
	mutex_exit(&timer14_p->mutex);			/* end MUTEX */

	return (0);
}


#define Validate_CPU(n) /*if(!VALID_CPU(n))goto nodev */
/*ARGSUSED*/
static	int
timer14_ioctl(dev_t dev, int cmd, int arg, int flag, cred_t *credp, int *rvalp)
{
	Timer14 *timer14_p;


	Timer14_status timer14_data,return_status;
	u_short		retval = 0;

	u_int counter_count;u_int cpu_mask,Cpu_No,dummy;

	timer14_p = (Timer14 *)ddi_get_soft_state(state_head, getminor(dev));
	mutex_enter(&timer14_p->mutex);

	retval = ENOTTY;

	if ((caddr_t)arg)
	    copyin((caddr_t)arg,(caddr_t)(&timer14_data), sizeof(struct timer14_status));
	return_status=timer14_data; /* make sure that unchanged fields are the same */
	switch(cmd){
      
		case TIMER14_ENQUIRE: /* return the current content of counter14 */
			timer14_data.intr_count=0; /* start uses cpu 0 by default */
		case TIMER14_ENQUIRE_SPECIFIC:
			Cpu_No=timer14_data.intr_count;
			Validate_CPU(Cpu_No);
			timer14_getcount(Cpu_No,&counter_count);
			if(counter_count&0x80000000){ 
				/* oh boy an interrupt has occured but not serviced yet */
				return_status.counter_reg = USECS(counter_count);
				return_status.counter_reg+=timer14_p->period[Cpu_No];
			}
			else if(timer14_p->counter_limit[Cpu_No]){
				/*
				 * Notice that if we are in this clause , 1)either when 
				 * we read the counter->counter14 it was just about     
				 * to reach the limit, in which case , interrupt        
				 * occured just after we read it, or 2)counter had      
     				 * already reached the limit but we had grabbed the mutex    
				 * so we have not incremented intr_count etc. yet.      
				 * Therefore to be safe we read the counter again....    
				 * THIS IS CRUCIAL..................................    
				*/
				timer14_getcount(Cpu_No,&counter_count);
				return_status.counter_reg = USECS(counter_count);
				return_status.counter_reg+=timer14_p->period[Cpu_No];
			}
			else{
				return_status.counter_reg = USECS(counter_count);
			}
			return_status.intr_count=timer14_p->interrupt_count[Cpu_No];
			return_status.interrupts_missed=timer14_p->interrupts_missed[Cpu_No];

			retval = 0;break;
		case TIMER14_INIT:   
		case TIMER14_START:     /* set counter limits, enable interrupts */
			timer14_data.intr_count=0; /* start uses cpu 0 by default */
		case TIMER14_START_SPECIFIC:
			Cpu_No=timer14_data.intr_count;
			Validate_CPU(Cpu_No);
			if (timer14_data.period <= MINPERIOD && timer14_data.period){
				cmn_err(CE_WARN,"Module Timer14 error: period below minimum %d\n", 
					MINPERIOD);
				retval = EIO;break;
			}
			
			timer14_p->Cpus_Active |= (1<<Cpu_No);

			INTR14_ON(Cpu_No);

			timer14_p->interrupt_count[Cpu_No]=0;timer14_p->interrupts_missed[Cpu_No]=0;
			timer14_setlimit(Cpu_No,timer14_data.period << CTR_USEC_SHIFT);
			timer14_p->period[Cpu_No]=timer14_data.period;

			return_status.intr_count=TIMER14_VERSION;

			retval = 0;break;

		case TIMER14_WAIT:
			timer14_data.intr_count=0; /* wait uses cpu 0 by default */
		case TIMER14_WAIT_SPECIFIC:
			Cpu_No=timer14_data.intr_count;
			Validate_CPU(Cpu_No);
			cpu_mask=(1<<Cpu_No);
			if(!timer14_p->interrupts_missed[Cpu_No]){
				do
					/* 
					if(!cv_wait_sig(&timer14_p->cv,&timer14_p->mutex)){
						goto bailout;
					}*/

					cv_wait(&timer14_p->cv,&timer14_p->mutex);
				while(!timer14_p->counter_triggered[Cpu_No]);
				timer14_p->interrupts_missed[Cpu_No]=0;
			}
			timer14_p->counter_triggered[Cpu_No]=0;
			timer14_getcount(Cpu_No,&counter_count);
			return_status.counter_reg = USECS(counter_count);
			return_status.intr_count=timer14_p->interrupt_count[Cpu_No];
			return_status.interrupts_missed=timer14_p->interrupts_missed[Cpu_No];
			timer14_p->interrupts_missed[Cpu_No]=0;
			retval = 0;break;


		
		case TIMER14_STOP:
			timer14_data.intr_count=0; /* stop uses cpu 0 by default */
		case TIMER14_STOP_SPECIFIC:
			Cpu_No=timer14_data.intr_count;
			Validate_CPU(Cpu_No);
			INTR14_OFF(Cpu_No);	 /* disable interrupts */
			timer14_p->Cpus_Active &= ~(1<<Cpu_No);
			retval = 0;break;

      		default:
			cmn_err(CE_WARN,"Module timer14: illegal command\n");
			break;
	}
	return_status.period=timer14_p->period[Cpu_No];
	mutex_exit(&timer14_p->mutex);
	copyout((caddr_t)(&return_status),(caddr_t)arg,sizeof(struct timer14_status));
	return (retval);

bailout:
	cmn_err(CE_WARN," BailOut %d",Cpu_No);
	mutex_exit(&timer14_p->mutex);
	return(EIO);
nodev:
	mutex_exit(&timer14_p->mutex);
	return(ENODEV);
	
}



/* soft interrupt handler for timer14 events.  We only need this because of
 * the call to cv_signal()
 */
static          u_int
                timer14_softintr(caddr_t arg)
{
	u_int           instance;
	Timer14        *timer14_p;
	register dev_info_t *dip = (dev_info_t *) arg;
	u_int           return_value=DDI_INTR_UNCLAIMED;
	u_int dummy;
	int Cpu_No,Local_Timer14_SRQ=0;

	swap_word(&Timer14_SRQ,&Local_Timer14_SRQ);

	if (Local_Timer14_SRQ) { 
		instance = ddi_get_instance(dip);
		timer14_p = (Timer14 *) ddi_get_soft_state(state_head, instance);
		if (timer14_p->attach_flags & MINOR_NODE_CREATED) {
			/* signal the waiting ioctl call */
			/*
			 * Now we must be careful.. If We are here after the
			 * ioctl(..,TIMER14_WAIT,... but before the actual
			 * cv_wait , then we will lose a tick. So therefore
			 * we attempt to grab the mutex. If it is not
			 * available then we wait until a cv_wait , or the
			 * mutex is released , otherwise we get the mutex,
			 * set a flag telling ioctl that we have traversed
			 * this path thus leaving it up to it do decide what
			 * to do next.
			 */

			mutex_enter(&timer14_p->mutex);	/* start MUTEX */
			{
				for(Cpu_No=0;Cpu_No< NUMBER_OF_CPUS;Cpu_No++){
					dummy=0;
					swap_word(&timer14_p->counter_limit[Cpu_No],&dummy);
					if( dummy){
						timer14_p->counter_triggered[Cpu_No]=1;
						timer14_p->interrupt_count[Cpu_No]++;
						timer14_p->interrupts_missed[Cpu_No]++;
						return_value = (DDI_INTR_CLAIMED);
					}
				}
				if(return_value==DDI_INTR_CLAIMED)cv_broadcast(&timer14_p->cv);
			}
			mutex_exit(&timer14_p->mutex);	/* end MUTEX */
		}

	}
	return (return_value);
}


/* DDI compliant interrupt handler, can be used if the ddi_add_intr()
 * supports addition of level 14 interrupts handlers  5.1 does not,  5.2 does.
 */

#define ISR_CHECK 	(MUTEX_ADDED | CV_ADDED | INTERRUPT_ADDED)

static u_int
timer14_intr(caddr_t arg)
{
    u_int 	int_serviced = DDI_INTR_UNCLAIMED;
    u_int	instance;
    Timer14 	*timer14_p;
    register dev_info_t	*dip = (dev_info_t *)arg;
    
	int counter_overflow,Cpu_No,Local_Timer14_SRQ=0,dummy=0;
    
    instance = ddi_get_instance(dip);
    timer14_p = (Timer14 *)ddi_get_soft_state(state_head, instance);
    
    
    if ((timer14_p->attach_flags & ISR_CHECK) == ISR_CHECK) { /* is everything set up yet? */

		timer14_p->flags &= ~TIMER14_DEV_BUSY;

		for(Cpu_No=0;Cpu_No< NUMBER_OF_CPUS;Cpu_No++){
			if( (1<<Cpu_No) & timer14_p->Cpus_Active){	
				/* reading limit register clears interrupt pending bit*/
				timer14_getlimit(Cpu_No,&counter_overflow);
				if(counter_overflow & 0x80000000){
					int_serviced = DDI_INTR_CLAIMED;
					timer14_p->counter_limit[Cpu_No]=1;
        			Local_Timer14_SRQ = 1;
				}
			}
		}
	}

    if(int_serviced==DDI_INTR_CLAIMED){
		Timer14_SRQ=Local_Timer14_SRQ;
		ddi_trigger_softintr(timer14_p->softintr_id);
	}
    return (int_serviced);
}


