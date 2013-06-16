#define HEIGHT 512
#define WIDTH 512
/*
struct Pixel{
	int place; 
	Pixel():place(0){};
};
*/
/*int place:
	- two digit number 
	- first digit: 
		- 1: corresponding to being in any of the layers Ln2, Ln1, Lz, Lp1, Lp2
		- 2: corresponding to being in any of the layers Sn2, Sn1, Sz, Sp1, Sp2 
	- second digit:
		- 3: Ln2 or Sn2
		- 4: Ln1 or Sn1
		- 5: Lz or Sz
		- 6: Lp1 or Sp1
		- 7: Lp2 or Sp2
	- Could have used two seperate integer values, but then a lot of additional 
	  checks in the code would be needed.
*/

