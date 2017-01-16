package main;

import java.io.*;
import java.nio.file.Files;
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

    public static String stripUnderscore(String source, String word) {
        String underscore = "__" + word + "__";
        System.out.println(source.indexOf("__signed__"));
        return source.replaceAll(underscore, word);
    }

    public static String fileToString(File file) throws IOException {
        byte[] bytes = Files.readAllBytes(file.toPath());
        return new String(bytes);
    }

    public static Process writeStringToProcess(ProcessBuilder processBuilder, String string) throws IOException {
        processBuilder.redirectInput(ProcessBuilder.Redirect.PIPE);
        Process process = processBuilder.start();
        process.getOutputStream().write(string.getBytes());
        process.getOutputStream().close();
        return process;
    }

    public static String preprocess(File file) throws IOException {
        //File uncommented = uncomment(file);
        File uncommented = file;

        String string = fileToString(file);

        //int firstLine = getFirstLine(uncommented);
        ProcessBuilder processBuilder = new ProcessBuilder("gcc", "-E", "-xc", "-");
        processBuilder.directory(file.getParentFile());
        Process process = writeStringToProcess(processBuilder, string);
        //processBuilder.redirectInput(uncommented);
        //Process process = processBuilder.start();
        //ProcessBuilder processBuilder = new ProcessBuilder("gcc", "-E", uncommented.getAbsolutePath());

        //Process process = processBuilder.start();
        Scanner scanner = new Scanner(process.getInputStream());
        Scanner scanner1 = new Scanner(process.getErrorStream());
        //Scanner scanner = new Scanner(file);
        //moveScannerToRealCode(scanner, file, firstLine);
        StringBuffer buffer = new StringBuffer();
        while (scanner.hasNextLine()) {
            String line = scanner.nextLine();
            if (!line.trim().startsWith("#")) {
                buffer.append(line + "\n");
            }
        }


        while (scanner1.hasNextLine()) {
            System.err.println(scanner1.nextLine());
        }
        String finalString = buffer.toString();
        finalString = stripUnderscore(finalString, "signed");
        //String finalString = removeFunctionAttributes(buffer.toString());
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
