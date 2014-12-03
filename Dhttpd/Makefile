all: dhttpd get post
cgi= cgi
dhttpd: dhttpd.c
	gcc -W -Wall -o dhttpd dhttpd.c -lpthread 
get: ${cgi}/get.c
	gcc $< -o ${cgi}/$@
post: ${cgi}/post.c
	gcc $< -o ${cgi}/$@
clean:
	rm dhttpd
