package main;

import java.io.*;
import java.util.Scanner;

/**
 * Created by ooee on 11/22/16.
 */
public class Preprocess {

    private static File uncomment(File file) throws IOException {
        Process process = Runtime.getRuntime().exec(new String[]{"gcc", "-fpreprocessed", "-dD", "-E", file.getAbsolutePath()});

        File tempFile = File.createTempFile("temp", ".c");
        byte[] bytes = new byte[1024];
        int read;
        FileOutputStream fileOutputStream = new FileOutputStream(tempFile);
        while ((read = process.getInputStream().read(bytes)) != -1) {
            fileOutputStream.write(bytes, 0, read);
        }
        fileOutputStream.close();
        return tempFile;
    }

    

    public static void moveScannerToRealCode(Scanner scanner, File file, int firstLine) {
        return;
        /*
        while (scanner.hasNextLine()) {
            String line = scanner.nextLine();
            if (line.startsWith("#") && line.contains(file.getName())) {
                Scanner lineScanner = new Scanner(line);
                if (lineScanner.hasNext() && lineScanner.next().equals("#")) {
                    if (lineScanner.hasNextInt() && lineScanner.nextInt() >= firstLine) {
                        if (lineScanner.hasNext() && lineScanner.next().equals(String.format("\"%s\"", file.getAbsolutePath()))) {
                            break;
                        }
                    }
                }
            }
        }
        */
    }

    public static String preprocess(File file) throws IOException {
        //File uncommented = uncomment(file);
        File uncommented = file;
        //int firstLine = getFirstLine(uncommented);
        //ProcessBuilder processBuilder = new ProcessBuilder("gcc", "-E", "-xc", "-");
        //processBuilder.redirectInput(uncommented);
        //Process process = processBuilder.start();
        ProcessBuilder processBuilder = new ProcessBuilder("gcc", "-I.", "-E", uncommented.getAbsolutePath());
        processBuilder.directory(file.getParentFile());
        Process process = processBuilder.start();
        Scanner scanner = new Scanner(process.getInputStream());
        //Scanner scanner = new Scanner(file);
        //moveScannerToRealCode(scanner, file, firstLine);
        StringBuffer buffer = new StringBuffer();
        while (scanner.hasNextLine()) {
            String line = scanner.nextLine();
            if (!line.trim().startsWith("#")) {
                buffer.append(line + "\n");
            }
        }
        String finalString = buffer.toString();
        //String finalString = removeFunctionAttributes(buffer.toString());
        //System.out.println(finalString);
        return finalString;
    }

    public static int getFirstLine(File file) throws FileNotFoundException {
        Scanner scanner = new Scanner(file);
        scanner.next();
        int startNumber = scanner.nextInt();
        int lineNumber = 0;
        int firstLineNumber = 0;
        while (scanner.hasNextLine()) {
            String line = scanner.nextLine();
            lineNumber += 1;
            if (line.startsWith("#")) {
                firstLineNumber = lineNumber;
            }
        }
        return firstLineNumber + startNumber - 1;
    }
}
