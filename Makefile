all: techshell

techshell: techshell.c
	gcc -o techshell techshell.c
	$(info  ∧　∧)
	$(info (*^∀^) done compiling!!)
	$(info  U￣U)

clean:
	rm -f techshell
