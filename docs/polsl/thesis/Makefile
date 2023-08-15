:
f=main
minted=
# minted=-shell-escape 
minteddir=_minted-$(f)

.PHONY : short
.PHONY : clean

default: pdflatex
.PHONY: default

xelatex : $(f).tex
	xelatex $(minted) -synctex=1 $(f)
	bibtex $(f)

pdflatex : $(f).tex
	@ pdflatex $(minted) -synctex=1 $(f)
	@ bibtex $(f)

$(f).pdf : $(f).tex
	pdflatex $(minted) -synctex=1 $(f)
	bibtex $(f)

short : $(f).tex
	pdflatex $(minted) -synctex=1 $(f)

clean : 
	@ for p in *.aux *.bbl *.bcf *.blg *.lof *.log *.lot *.out *.toc *.xml *.gz *-blx.bib ; \
	  do \
		if [ -f $$p ] ; then rm $$p; fi; \
	done;  \
	if [ -d $(minteddir) ] ; then rm -r $(minteddir) ; fi; 
