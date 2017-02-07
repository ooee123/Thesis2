package main;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.file.Files;

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

    

    public static String skipNLines(String string, int firstLine) {
        Scanner scanner = new Scanner(string);
        for (int i = 0; i < firstLine; i++) {
            scanner.nextLine();
        }
        StringBuilder builder = new StringBuilder();
        while (scanner.hasNextLine()) {
            String line = scanner.nextLine();
            builder.append(line + "\n");
        }
        return builder.toString();
    }

    public static String stripUnderscore(String source, String word) {
        String underscore = "__" + word + "__";
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

    public static String prependUndef(String string) {
        string = string.replaceAll("FD_SET\\(", "\n#undef FD_SET\nFD_SET(");
        string = string.replaceAll("FD_ZERO\\(", "\n#undef FD_ZERO\nFD_ZERO(");
        string = string.replaceAll("FD_ISSET\\(", "\n#undef FD_ISSET\nFD_ISSET(");
        string = string.replaceAll("FD_CLR\\(", "\n#undef FD_CLR\nFD_CLR(");
        return string;
    }

    public static String preprocess(File file) throws IOException {
        //File uncommented = uncomment(file);
        File uncommented = file;

        String string = fileToString(file);
        string = prependUndef(string);
        //int firstLine = getFirstLine(uncommented);
        ProcessBuilder processBuilder = new ProcessBuilder("gcc", "-E", "-xc", "-");
        processBuilder.directory(file.getParentFile());
        Process process = writeStringToProcess(processBuilder, string);
        //processBuilder.redirectInput(uncommented);
        //Process process = processBuilder.start();
        //ProcessBuilder processBuilder = new ProcessBuilder("gcc", "-E", uncommented.getAbsolutePath());

        //Process process = processBuilder.start();
        Scanner scanner = new Scanner(process.getInputStream());
        //Scanner scanner = new Scanner(file);
        //moveScannerToRealCode(scanner, file, firstLine);
        StringBuffer buffer = new StringBuffer();
        while (scanner.hasNextLine()) {
            String line = scanner.nextLine();
            //if (!line.trim().startsWith("#")) {
                buffer.append(line + "\n");
            //}
        }

        Scanner scanner1 = new Scanner(process.getErrorStream());
        while (scanner1.hasNextLine()) {
            System.err.println(scanner1.nextLine());
        }
        String finalString = buffer.toString();
        finalString = stripUnderscore(finalString, "signed");
        //int firstLine = getFirstLine(finalString, file.getName());
        int firstLine = getFirstLine(finalString, "<stdin>");
        String originalCode = skipNLines(finalString, firstLine);
        //String finalString = removeFunctionAttributes(buffer.toString());
        return finalString;
    }

    public static int getSplittingIndex(String string) {
        int i = string.lastIndexOf("\"<stdin>\" 2");
        while (string.charAt(i) != '#') {
            i -= 1;
        }
        return i;
    }

    public static String removePounds(String string) {
        Scanner scanner = new Scanner(string);
        StringBuffer buffer = new StringBuffer();
        while (scanner.hasNextLine()) {
            String line = scanner.nextLine();
            if (!line.trim().startsWith("#")) {
                buffer.append(line + "\n");
            }
        }
        return buffer.toString();
    }

    public static int getFirstLine(String string, String fileName) throws FileNotFoundException {
        int firstLine = 0;
        int lineNumber = 0;
        Scanner scanner = new Scanner(string);
        while (scanner.hasNextLine()) {
            lineNumber += 1;
            String line = scanner.nextLine();
            if (line.startsWith("#")) {
                Scanner lineScanner = new Scanner(line);
                lineScanner.next("#");
                if (lineScanner.hasNextInt()) {
                    lineScanner.nextInt();
                    String scannedFileName = lineScanner.next();
                    if (scannedFileName.equals("\"" + fileName + "\"")) {
                        while (lineScanner.hasNextInt()) {
                            int flag = lineScanner.nextInt();
                            if (flag == 2) {
                                firstLine = lineNumber;
                            }
                        }
                    }
                }
            }
        }
        return firstLine;
    }

    public static String insertUndefs(String string, int firstLine) {
        Scanner scanner = new Scanner(string);
        ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream();
        PrintWriter printer = new PrintWriter(byteArrayOutputStream);
        // Print until end of preprocessor directives
        for (int i = 0; i < firstLine; i++) {
            printer.println(scanner.nextLine());
        }
        /*
        while (scanner.hasNextLine()) {
            printer.println(scanner.nextLine());
        }
        */
        printer.println("#undef FD_SET");
        printer.println("#undef FD_ISSET");
        printer.println("#undef FD_ZERO");
        while (scanner.hasNextLine()) {
            printer.println(scanner.nextLine());
        }
        printer.close();
        return byteArrayOutputStream.toString();
    }
}