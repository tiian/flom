BEGIN { 
	FS=","
	VALUE=0
	print "package  org.tiian.flom;"
	print "/**"
    print " * This class contains the constants necessary to map the"
    print " * lock modes used by the C native functions wrapped by the JNI"
    print " * methods."
	print " * See C header file src/flom_types.h for a verbose description"
	print " * of every lock mode."
	print " */"
	print "public class FlomLockModes {" 
}
/FLOM_LOCK_MODE_/	{ 
	print "\tpublic final static int " $1 " = " VALUE ";" 
	VALUE++
}
END   { 
	print "}"
}
