BEGIN { 
	print "package  org.tiian.flom;"
	print "public class FlomErrorCodes {" 
}
/#define.*FLOM_/	{ print "\tpublic final static int " $2 " = " $3 ";" }
END   { 
	print "\tpublic native static String getText(int code);"
	print "}"
}
