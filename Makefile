JAVAC=/home/kly04/Downloads/jdk1.8.0_101/bin/javac

all:
	$(JAVAC) -cp .:\* -d build/ ast/*.java ast/type/*.java parser/*.java visitor/*.java pdg/*.java

clean:
	rm -r build/*
