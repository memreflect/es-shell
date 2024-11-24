/* prim.h -- definitions for es primitives ($Revision: 1.1.1.1 $) */

#define	PRIM(name)	static List *CONCAT(prim_,name)( \
				List UNUSED *list, Binding UNUSED *binding, int UNUSED evalflags \
			)
#define	X(name)		addprim( \
				STRING(name), \
				CONCAT(prim_,name) \
			)

extern void initprims_controlflow(void);	/* prim-ctl.c */
extern void initprims_io(void);			/* prim-io.c */
extern void initprims_etc(void);		/* prim-etc.c */
extern void initprims_sys(void);		/* prim-sys.c */
extern void initprims_proc(void);		/* proc.c */
extern void initprims_access(void);		/* access.c */
