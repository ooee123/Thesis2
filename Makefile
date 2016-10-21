JAVAC=javac

all:
	$(JAVAC) -cp .:\* -d build/ ast/*.java ast/type/*.java parser/*.java visitor/*.java pdg/*.java main/*.java

clean:
	rm -r build/*
