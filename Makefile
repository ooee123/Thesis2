JAVAC=/home/kly04/Downloads/jdk1.8.0_101/bin/javac

all:
	$(JAVAC) -cp .:\* -d build/ ast/*.java ast/declaration/*.java ast/type/*.java parser/*.java visitor/*.java pdg/*.java main/*.java IdentifierNormalizer/*.java 

clean:
	rm -r build/*
