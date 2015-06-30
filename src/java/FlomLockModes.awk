BEGIN { 
	FS=","
	VALUE=0
	print "package  org.tiian.flom;"
	print "/**"
	print " * See C header file src/flom_types.h to discover the meaning"
	print " * of every lock mode"
	print " */"
	print "class FlomLockModes {" 
}
/FLOM_LOCK_MODE_/	{ 
	print "\tpublic final static int " $1 " = " VALUE ";" 
	VALUE++
}
END   { 
	print "}"
}
