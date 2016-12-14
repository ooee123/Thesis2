package main;

import com.sun.xml.internal.messaging.saaj.util.ByteOutputStream;

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

    private static String removeFunctionAttributes(String string) {
        string = string.replaceAll("__attribute__", "");
        string = string.replaceAll("\\(\\(__packed__\\)\\)", "");
        return string;
    }

    private void removeFunctionAttributes(Scanner scanner) {
        StringBuilder builder = new StringBuilder();
        String attribute = scanner.findInLine("__attribute__");
        while (attribute != null) {
            scanner.next("__attribute__");
            int remaining = 0;
            while (remaining < 2) {
                char c = scanner.next(".").charAt(0);
                if (c == '(') {
                    remaining++;
                } else if (c == ')') {
                    remaining--;
                }
            }
            while (remaining > 0) {
                char c = scanner.next(".").charAt(0);
                if (c == '(') {
                    remaining++;
                } else if (c == ')') {
                    remaining--;
                }
            }
        }
    }

    public static void moveScannerToRealCode(Scanner scanner, File file, int firstLine) {
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
    }

    public static String fileToString(File file) throws IOException {
        InputStream inputStream = new FileInputStream(file);
        ByteArrayOutputStream byteOutputStream = new ByteArrayOutputStream();
        int byteRead;
        while ((byteRead = inputStream.read()) != -1) {
            byteOutputStream.write(byteRead);
        }
        byteOutputStream.close();
        return byteOutputStream.toString();
    }

    public static String proprocess(File file) throws IOException {
        //File uncommented = uncomment(file);
        File uncommented = file;
        String fileString = fileToString(file);
        int firstLine = getFirstLine(fileString);
        fileString = insertUndefs(fileString, firstLine);
        //ProcessBuilder processBuilder = new ProcessBuilder("gcc", "-E", "-xc", "-");
        //processBuilder.redirectInput(uncommented);
        //Process process = processBuilder.start();
        //ProcessBuilder processBuilder = new ProcessBuilder("gcc", "-I.", "-E", uncommented.getAbsolutePath());
        ProcessBuilder processBuilder = new ProcessBuilder("gcc", "-E", "-xc", "-");
        processBuilder.redirectInput(ProcessBuilder.Redirect.PIPE);
        processBuilder.directory(file.getParentFile());
        Process process = processBuilder.start();
        OutputStream pipeToProcessStdin = process.getOutputStream();
        pipeToProcessStdin.write(fileString.getBytes());
        pipeToProcessStdin.close();
        Scanner scanner = new Scanner(process.getInputStream());
        moveScannerToRealCode(scanner, file, firstLine);
        StringBuffer buffer = new StringBuffer();
        while (scanner.hasNextLine()) {
            String line = scanner.nextLine();
            if (!line.trim().startsWith("#")) {
                buffer.append(line);
            }
        }
        //String finalString = removeFunctionAttributes(buffer.toString());
        return buffer.toString();
    }

    public static int getFirstLine(String string) throws FileNotFoundException {
        Scanner scanner = new Scanner(string);
        // Why next?
        //scanner.next();
        //int startNumber = scanner.nextInt();
        int lineNumber = 0;
        int firstLineNumber = 0;
        while (scanner.hasNextLine()) {
            String line = scanner.nextLine();
            lineNumber += 1;
            if (line.startsWith("#")) {
                firstLineNumber = lineNumber;
            }
        }
        return firstLineNumber;
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
