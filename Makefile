JAVAC=javac

all:
	$(JAVAC) -cp .:\* -d build/ ast/*.java ast/statement/*.java ast/expression/*.java ast/expression/impl/*.java ast/statement/impl/*.java ast/declaration/*.java ast/type/*.java parser/*.java visitor/*.java pdg/*.java main/*.java IdentifierNormalizer/*.java 

clean:
	rm -r build/*
