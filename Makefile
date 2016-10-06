all:
	javac -cp .:\* -d build/ ast/*.java ast/type/*.java parser/*.java visitor/*.java

clean:
	rm -r build/*
