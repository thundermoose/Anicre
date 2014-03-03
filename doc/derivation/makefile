TEX = pdflatex -interaction nonstopmode
BIB = bibtex
GS = gs -q -dNOPAUSE -dBATCH -sDEVICE=pdfwrite 
 
PAPER = trdens
BIBFILE = temp.bib
BUNDLE = trdens.pdf
 
all: $(PAPER).pdf 
#	$(GS) -sOutputFile=$(BUNDLE) $(PAPER).pdf
 
view: $(BUNDLE)
	xpdf $(BUNDLE) &
 
clean::
	rm -fv *.aux *.log *.bbl *.blg *.toc *.out *.lot *.lof $(PAPER).pdf

$(PAPER).pdf: $(PAPER).tex #$(PAPER).bbl 
	$(TEX) $(PAPER) 
	$(TEX) $(PAPER)
 
#$(PAPER).bbl: $(PAPER).tex $(BIBFILE)
#	$(TEX) $(PAPER)
#	$(BIB) $(PAPER)
