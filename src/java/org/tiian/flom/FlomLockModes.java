package  org.tiian.flom;
/**
 * This class contains the constants necessary to map the
 * lock modes used by the C native functions wrapped by the JNI
 * methods.
 * See C header file src/flom_types.h for a verbose description
 * of every lock mode.
 */
public class FlomLockModes {
	/** Constant for lock mode     FLOM_LOCK_MODE_NL */
	public final static int     FLOM_LOCK_MODE_NL = 0;
	/** Constant for lock mode     FLOM_LOCK_MODE_CR */
	public final static int     FLOM_LOCK_MODE_CR = 1;
	/** Constant for lock mode     FLOM_LOCK_MODE_CW */
	public final static int     FLOM_LOCK_MODE_CW = 2;
	/** Constant for lock mode     FLOM_LOCK_MODE_PR */
	public final static int     FLOM_LOCK_MODE_PR = 3;
	/** Constant for lock mode     FLOM_LOCK_MODE_PW */
	public final static int     FLOM_LOCK_MODE_PW = 4;
	/** Constant for lock mode     FLOM_LOCK_MODE_EX */
	public final static int     FLOM_LOCK_MODE_EX = 5;
	/** Constant for lock mode     FLOM_LOCK_MODE_N */
	public final static int     FLOM_LOCK_MODE_N = 6;
	/** Constant for lock mode     FLOM_LOCK_MODE_INVALID */
	public final static int     FLOM_LOCK_MODE_INVALID = 7;
}
