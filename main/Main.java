package main;

import IdentifierNormalizer.ProgramIdentifierNormalizerVisitor;
import ast.Function;
import ast.Program;
import ast.statement.impl.CompoundStatement;
import org.antlr.v4.runtime.ANTLRInputStream;
import org.antlr.v4.runtime.CommonTokenStream;
import parser.CLexer;
import parser.CParser;
import pdg.*;
import visitor.PDGGenerationVisitor;
import visitor.PrimaryExpressionTypeAssignerVisitor;
import visitor.TreeToASTVisitor;

import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Iterator;

/**
 * Created by ooee on 10/20/16.
 */
public class Main {
    public static boolean SHOW_ORIGINAL_LINE = true;
    public static boolean NORMALIZE_IDENTIFIERS = true;
    public static boolean SORT_STATEMENTS = true;
    public static boolean REMOVE_USELESS_CODE = true;
    public static boolean SPLIT_FUNCTIONS_MOSS = false;
    public static void main(String args[]) {
        if (args.length < 1) {
            System.err.println("Usage: [-cisrm] <C source file>");
            System.err.println("c for original line as comments");
            System.err.println("i for normalizing identifiers");
            System.err.println("s for sort statements");
            System.err.println("r for removing useless code. s must be turned on for this feature");
            System.err.println("m for moss mode. If specified, you must provide a directory to put all the files");
            System.exit(-1);
        }
        String mossDirectory = null;
        String sourceFile = args[0];
        if (args[0].startsWith("-")) {
            SHOW_ORIGINAL_LINE = args[0].contains("c");
            NORMALIZE_IDENTIFIERS = args[0].contains("i");
            SORT_STATEMENTS = args[0].contains("s");
            REMOVE_USELESS_CODE = args[0].contains("r");
            SPLIT_FUNCTIONS_MOSS = args[0].contains("m");
            if (SPLIT_FUNCTIONS_MOSS) {
                if (args.length < 2) {
                    System.err.println("You must provide a directory after -m to put the moss files");
                    System.exit(-1);
                } else {
                    mossDirectory = args[1];
                    sourceFile = args[2];
                }
            } else {
                sourceFile = args[1];
            }
        }
        System.err.println("Starting on " + sourceFile);
        try {
            String preprocess = Preprocess.preprocess(new File(sourceFile));
            int splittingIndex = Preprocess.getSplittingIndex(preprocess);
            String firstPart = Preprocess.removePounds(preprocess.substring(0, splittingIndex));
            String secondPart = Preprocess.removePounds(preprocess.substring(splittingIndex, preprocess.length()));
            //ByteArrayInputStream firstPartByteArrayInputStream = new ByteArrayInputStream(Preprocess.preprocess(new File(args[0])).getBytes());
            ByteArrayInputStream firstPartByteArrayInputStream = new ByteArrayInputStream(firstPart.getBytes());
            ByteArrayInputStream secondPartByteArrayInputStream = new ByteArrayInputStream(secondPart.getBytes());
            CLexer firstPartlexer = new CLexer(new ANTLRInputStream(firstPartByteArrayInputStream));
            CLexer secondPartlexer = new CLexer(new ANTLRInputStream(secondPartByteArrayInputStream));
            CommonTokenStream firstPartTokens = new CommonTokenStream(firstPartlexer);
            CommonTokenStream secondPartTokens = new CommonTokenStream(secondPartlexer);
            System.err.print("Begin parsing " + sourceFile + "...");
            CParser firstPartParser = new CParser(firstPartTokens);
            CParser secondPartParser = new CParser(secondPartTokens);
            System.err.println("Done");
            CParser.CompilationUnitContext firstPartCompilationUnit = firstPartParser.compilationUnit();
            CParser.CompilationUnitContext secondPartCompilationUnit = secondPartParser.compilationUnit();
            TreeToASTVisitor visitor = new TreeToASTVisitor(firstPartCompilationUnit, secondPartTokens);
            System.err.print("Begin AST building...");
            Program program = visitor.visit(secondPartCompilationUnit);
            System.err.println("Done");
            System.err.print("Removing bswaps...");
            Iterator<Function> iterator = program.getFunction().iterator();
            while (iterator.hasNext()) {
                Function next = iterator.next();
                if (next.getIdentifier().equals("__bswap_32")) {
                    iterator.remove();
                } else if (next.getIdentifier().equals("__bswap_64")) {
                    iterator.remove();
                }
            }
            System.err.println("Done");

            System.err.print("Assigning types to primary expressions...");
            new PrimaryExpressionTypeAssignerVisitor(program);
            System.err.println("Done");

            PDGGenerationVisitor pdgGenerationVisitor = new PDGGenerationVisitor(program);

            //pdgGenerationVisitor.visit(program);
            //FunctionArgumentOrderVisitor functionArgumentOrderVisitor = new FunctionArgumentOrderVisitor();
            //functionArgumentOrderVisitor.visit(program);
            if (SORT_STATEMENTS) {
                for (Function function : program.getFunction()) {
                    System.err.print("Begin PDG generation for function " + function.getIdentifier() + "...");
                    PDGNodeCompoundStatement functionBody = pdgGenerationVisitor.visit(function);
                    System.err.println("Done");
                    if (REMOVE_USELESS_CODE) {
                        System.err.print("Begin Useless removal for function " + function.getIdentifier() + "...");
                        PDGUselessCodeRemover pdgUselessCodeRemover = new PDGUselessCodeRemover();
                        pdgUselessCodeRemover.removeUselessCode(functionBody);
                        System.err.println("Done");
                    }
                    PDGNodeTransitiveReducer pdgNodeTransitiveReducer = new PDGNodeTransitiveReducer();
                    pdgNodeTransitiveReducer.reduce(functionBody);
                    System.err.print("Begin serializing PDG for function " + function.getIdentifier() + "...");
                    PDGSorter sorter = new PDGSorterDefault();
                    CompoundStatement statement = sorter.sort(functionBody.getBody());
                    System.err.println("Done");
                    function.setCompoundStatement(statement);
                }
            }
            if (NORMALIZE_IDENTIFIERS) {
                System.err.print("Normalizing identfiers...");
                ProgramIdentifierNormalizerVisitor programIdentifierNormalizerVisitor = new ProgramIdentifierNormalizerVisitor();
                programIdentifierNormalizerVisitor.visit(program);
                System.err.println("Done");
            }
            if (SPLIT_FUNCTIONS_MOSS) {
                System.err.print("Splitting functions...");
                for (Function function : program.getFunction()) {
                    String functionCode = function.toCode(SHOW_ORIGINAL_LINE);
                    File functionFile = new File(mossDirectory, sourceFile + "_" + function.getIdentifier() + ".c");
                    FileWriter fileWriter = new FileWriter(functionFile);
                    fileWriter.write(functionCode);
                    fileWriter.close();
                }
                System.err.println("Done");
            }
            System.out.println(program.toCode(SHOW_ORIGINAL_LINE));
            System.err.println("Finish on " + sourceFile);
        } catch (IOException e) {
            System.err.println(e);
        }
    }
}
