#if defined(__arm64__)
/*
 * IDENTIFICATION:
 * stub generated <suppressed by mig_fix.py>
 * OPTIONS: 
 */
#define	__MIG_check__Reply__child_port_subsystem__ 1

#include "child_port.h"

/* TODO: #include <mach/mach.h> */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
extern void mach_msg_destroy(mach_msg_header_t *);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#ifndef	mig_internal
#define	mig_internal	static __inline__
#endif	/* mig_internal */

#ifndef	mig_external
#define mig_external
#endif	/* mig_external */

#if	!defined(__MigTypeCheck) && defined(TypeCheck)
#define	__MigTypeCheck		TypeCheck	/* Legacy setting */
#endif	/* !defined(__MigTypeCheck) */

#if	!defined(__MigKernelSpecificCode) && defined(_MIG_KERNEL_SPECIFIC_CODE_)
#define	__MigKernelSpecificCode	_MIG_KERNEL_SPECIFIC_CODE_	/* Legacy setting */
#endif	/* !defined(__MigKernelSpecificCode) */

#ifndef	LimitCheck
#define	LimitCheck 0
#endif	/* LimitCheck */

#ifndef	min
#define	min(a,b)  ( ((a) < (b))? (a): (b) )
#endif	/* min */

#if !defined(_WALIGN_)
#define _WALIGN_(x) (((x) + 3) & ~3)
#endif /* !defined(_WALIGN_) */

#if !defined(_WALIGNSZ_)
#define _WALIGNSZ_(x) _WALIGN_(sizeof(x))
#endif /* !defined(_WALIGNSZ_) */

#ifndef	UseStaticTemplates
#define	UseStaticTemplates	0
#endif	/* UseStaticTemplates */

#ifndef MIG_SERVER_ROUTINE
#define MIG_SERVER_ROUTINE
#endif

#ifndef	__MachMsgErrorWithTimeout
#define	__MachMsgErrorWithTimeout(_R_) { \
	switch (_R_) { \
	case MACH_SEND_INVALID_DATA: \
	case MACH_SEND_INVALID_DEST: \
	case MACH_SEND_INVALID_HEADER: \
		mig_put_reply_port(InP->Head.msgh_reply_port); \
		break; \
	case MACH_SEND_TIMED_OUT: \
	case MACH_RCV_TIMED_OUT: \
	default: \
		mig_dealloc_reply_port(InP->Head.msgh_reply_port); \
	} \
}
#endif	/* __MachMsgErrorWithTimeout */

#ifndef	__MachMsgErrorWithoutTimeout
#define	__MachMsgErrorWithoutTimeout(_R_) { \
	switch (_R_) { \
	case MACH_SEND_INVALID_DATA: \
	case MACH_SEND_INVALID_DEST: \
	case MACH_SEND_INVALID_HEADER: \
		mig_put_reply_port(InP->Head.msgh_reply_port); \
		break; \
	default: \
		mig_dealloc_reply_port(InP->Head.msgh_reply_port); \
	} \
}
#endif	/* __MachMsgErrorWithoutTimeout */

#ifndef	__DeclareSendRpc
#define	__DeclareSendRpc(_NUM_, _NAME_)
#endif	/* __DeclareSendRpc */

#ifndef	__BeforeSendRpc
#define	__BeforeSendRpc(_NUM_, _NAME_)
#endif	/* __BeforeSendRpc */

#ifndef	__AfterSendRpc
#define	__AfterSendRpc(_NUM_, _NAME_)
#endif	/* __AfterSendRpc */

#ifndef	__DeclareSendSimple
#define	__DeclareSendSimple(_NUM_, _NAME_)
#endif	/* __DeclareSendSimple */

#ifndef	__BeforeSendSimple
#define	__BeforeSendSimple(_NUM_, _NAME_)
#endif	/* __BeforeSendSimple */

#ifndef	__AfterSendSimple
#define	__AfterSendSimple(_NUM_, _NAME_)
#endif	/* __AfterSendSimple */

#define msgh_request_port	msgh_remote_port
#define msgh_reply_port		msgh_local_port



/* SimpleRoutine child_port_check_in */
mig_external kern_return_t child_port_check_in
(
	child_port_server_t server,
	child_port_token_t token,
	mach_port_t port,
	mach_msg_type_name_t portPoly
)
{

#ifdef  __MigPackStructs
#pragma pack(push, 4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t port;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		child_port_token_t token;
	} Request __attribute__((unused));
#ifdef  __MigPackStructs
#pragma pack(pop)
#endif
	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	union {
		Request In;
	} Mess;

	Request *InP = &Mess.In;

	mach_msg_return_t msg_result;

#ifdef	__MIG_check__Reply__child_port_check_in_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Reply__child_port_check_in_t__defined */

	__DeclareSendSimple(10011, "child_port_check_in")

#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t portTemplate = {
		/* name = */		MACH_PORT_NULL,
		/* pad1 = */		0,
		/* pad2 = */		0,
		/* disp = */		((mach_msg_type_name_t) -1),
		/* type = */		MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

	InP->msgh_body.msgh_descriptor_count = 1;
#if	UseStaticTemplates
	InP->port = portTemplate;
	InP->port.name = port;
	InP->port.disposition = portPoly;
#else	/* UseStaticTemplates */
	InP->port.name = port;
	InP->port.disposition = portPoly;
	InP->port.type = MACH_MSG_PORT_DESCRIPTOR;
#endif	/* UseStaticTemplates */

	InP->NDR = NDR_record;

	InP->token = token;

	InP->Head.msgh_reply_port = MACH_PORT_NULL;
	InP->Head.msgh_bits = MACH_MSGH_BITS_COMPLEX|
		MACH_MSGH_BITS(19, 0);
	/* msgh_size passed as argument */
	InP->Head.msgh_request_port = server;
	InP->Head.msgh_id = 10011;
	InP->Head.msgh_reserved = 0;
	
/* BEGIN VOUCHER CODE */

#ifdef USING_VOUCHERS
	if (voucher_mach_msg_set != NULL) {
		voucher_mach_msg_set(&InP->Head);
	}
#endif // USING_VOUCHERS
	
/* END VOUCHER CODE */

	__BeforeSendSimple(10011, "child_port_check_in")
	msg_result = mach_msg(&InP->Head, MACH_SEND_MSG|MACH_MSG_OPTION_NONE, (mach_msg_size_t)sizeof(Request), 0, MACH_PORT_NULL, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
	__AfterSendSimple(10011, "child_port_check_in")
	return msg_result;
}

#endif  /* __arm64__ */
#if defined(__x86_64__)
/*
 * IDENTIFICATION:
 * stub generated <suppressed by mig_fix.py>
 * OPTIONS: 
 */
#define	__MIG_check__Reply__child_port_subsystem__ 1

#include "child_port.h"

/* TODO: #include <mach/mach.h> */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
extern void mach_msg_destroy(mach_msg_header_t *);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#ifndef	mig_internal
#define	mig_internal	static __inline__
#endif	/* mig_internal */

#ifndef	mig_external
#define mig_external
#endif	/* mig_external */

#if	!defined(__MigTypeCheck) && defined(TypeCheck)
#define	__MigTypeCheck		TypeCheck	/* Legacy setting */
#endif	/* !defined(__MigTypeCheck) */

#if	!defined(__MigKernelSpecificCode) && defined(_MIG_KERNEL_SPECIFIC_CODE_)
#define	__MigKernelSpecificCode	_MIG_KERNEL_SPECIFIC_CODE_	/* Legacy setting */
#endif	/* !defined(__MigKernelSpecificCode) */

#ifndef	LimitCheck
#define	LimitCheck 0
#endif	/* LimitCheck */

#ifndef	min
#define	min(a,b)  ( ((a) < (b))? (a): (b) )
#endif	/* min */

#if !defined(_WALIGN_)
#define _WALIGN_(x) (((x) + 3) & ~3)
#endif /* !defined(_WALIGN_) */

#if !defined(_WALIGNSZ_)
#define _WALIGNSZ_(x) _WALIGN_(sizeof(x))
#endif /* !defined(_WALIGNSZ_) */

#ifndef	UseStaticTemplates
#define	UseStaticTemplates	0
#endif	/* UseStaticTemplates */

#ifndef MIG_SERVER_ROUTINE
#define MIG_SERVER_ROUTINE
#endif

#ifndef	__MachMsgErrorWithTimeout
#define	__MachMsgErrorWithTimeout(_R_) { \
	switch (_R_) { \
	case MACH_SEND_INVALID_DATA: \
	case MACH_SEND_INVALID_DEST: \
	case MACH_SEND_INVALID_HEADER: \
		mig_put_reply_port(InP->Head.msgh_reply_port); \
		break; \
	case MACH_SEND_TIMED_OUT: \
	case MACH_RCV_TIMED_OUT: \
	default: \
		mig_dealloc_reply_port(InP->Head.msgh_reply_port); \
	} \
}
#endif	/* __MachMsgErrorWithTimeout */

#ifndef	__MachMsgErrorWithoutTimeout
#define	__MachMsgErrorWithoutTimeout(_R_) { \
	switch (_R_) { \
	case MACH_SEND_INVALID_DATA: \
	case MACH_SEND_INVALID_DEST: \
	case MACH_SEND_INVALID_HEADER: \
		mig_put_reply_port(InP->Head.msgh_reply_port); \
		break; \
	default: \
		mig_dealloc_reply_port(InP->Head.msgh_reply_port); \
	} \
}
#endif	/* __MachMsgErrorWithoutTimeout */

#ifndef	__DeclareSendRpc
#define	__DeclareSendRpc(_NUM_, _NAME_)
#endif	/* __DeclareSendRpc */

#ifndef	__BeforeSendRpc
#define	__BeforeSendRpc(_NUM_, _NAME_)
#endif	/* __BeforeSendRpc */

#ifndef	__AfterSendRpc
#define	__AfterSendRpc(_NUM_, _NAME_)
#endif	/* __AfterSendRpc */

#ifndef	__DeclareSendSimple
#define	__DeclareSendSimple(_NUM_, _NAME_)
#endif	/* __DeclareSendSimple */

#ifndef	__BeforeSendSimple
#define	__BeforeSendSimple(_NUM_, _NAME_)
#endif	/* __BeforeSendSimple */

#ifndef	__AfterSendSimple
#define	__AfterSendSimple(_NUM_, _NAME_)
#endif	/* __AfterSendSimple */

#define msgh_request_port	msgh_remote_port
#define msgh_reply_port		msgh_local_port



/* SimpleRoutine child_port_check_in */
mig_external kern_return_t child_port_check_in
(
	child_port_server_t server,
	child_port_token_t token,
	mach_port_t port,
	mach_msg_type_name_t portPoly
)
{

#ifdef  __MigPackStructs
#pragma pack(push, 4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t port;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		child_port_token_t token;
	} Request __attribute__((unused));
#ifdef  __MigPackStructs
#pragma pack(pop)
#endif
	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	union {
		Request In;
	} Mess;

	Request *InP = &Mess.In;

	mach_msg_return_t msg_result;

#ifdef	__MIG_check__Reply__child_port_check_in_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Reply__child_port_check_in_t__defined */

	__DeclareSendSimple(10011, "child_port_check_in")

#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t portTemplate = {
		/* name = */		MACH_PORT_NULL,
		/* pad1 = */		0,
		/* pad2 = */		0,
		/* disp = */		((mach_msg_type_name_t) -1),
		/* type = */		MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

	InP->msgh_body.msgh_descriptor_count = 1;
#if	UseStaticTemplates
	InP->port = portTemplate;
	InP->port.name = port;
	InP->port.disposition = portPoly;
#else	/* UseStaticTemplates */
	InP->port.name = port;
	InP->port.disposition = portPoly;
	InP->port.type = MACH_MSG_PORT_DESCRIPTOR;
#endif	/* UseStaticTemplates */

	InP->NDR = NDR_record;

	InP->token = token;

	InP->Head.msgh_reply_port = MACH_PORT_NULL;
	InP->Head.msgh_bits = MACH_MSGH_BITS_COMPLEX|
		MACH_MSGH_BITS(19, 0);
	/* msgh_size passed as argument */
	InP->Head.msgh_request_port = server;
	InP->Head.msgh_id = 10011;
	InP->Head.msgh_reserved = 0;
	
/* BEGIN VOUCHER CODE */

#ifdef USING_VOUCHERS
	if (voucher_mach_msg_set != NULL) {
		voucher_mach_msg_set(&InP->Head);
	}
#endif // USING_VOUCHERS
	
/* END VOUCHER CODE */

	__BeforeSendSimple(10011, "child_port_check_in")
	msg_result = mach_msg(&InP->Head, MACH_SEND_MSG|MACH_MSG_OPTION_NONE, (mach_msg_size_t)sizeof(Request), 0, MACH_PORT_NULL, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
	__AfterSendSimple(10011, "child_port_check_in")
	return msg_result;
}

#endif  /* __x86_64__ */
