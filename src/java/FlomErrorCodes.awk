BEGIN { 
	print "package  org.tiian.flom;"
	print "class FlomErrorCodes {" 
}
/#define.*FLOM_/	{ print "\tpublic final static int " $2 " = " $3 ";" }
END   { print "}" }
