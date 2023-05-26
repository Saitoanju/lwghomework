OBJS=printall.c 
all:printall 

printall: ${OBJS}
	gcc ${OBJS} -o $@ -lnids -lnet -lpcap -lgthread-2.0 -lglib-2.0 -Wall -g
# g++ printall.cpp -o printall -lnids -lnet -lpcap -lgthread-2.0 -lglib-2.0 -Wall -g

# test:test.c
# 	gcc test.c -o test -lpcap -lnids -lnet -lgthread-2.0 -Wall -g
	

clean:
	rm printall 
# rm test 