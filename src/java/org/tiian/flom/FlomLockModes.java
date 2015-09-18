package  org.tiian.flom;
/**
 * This class contains the constants necessary to map the
 * lock modes used by the C native functions wrapped by the JNI
 * methods.
 * See C header file src/flom_types.h for a verbose description
 * of every lock mode.
 */
public class FlomLockModes {
	public final static int     FLOM_LOCK_MODE_NL = 0;
	public final static int     FLOM_LOCK_MODE_CR = 1;
	public final static int     FLOM_LOCK_MODE_CW = 2;
	public final static int     FLOM_LOCK_MODE_PR = 3;
	public final static int     FLOM_LOCK_MODE_PW = 4;
	public final static int     FLOM_LOCK_MODE_EX = 5;
	public final static int     FLOM_LOCK_MODE_N = 6;
	public final static int     FLOM_LOCK_MODE_INVALID = 7;
}
