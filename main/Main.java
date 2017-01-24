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
import java.io.IOException;

/**
 * Created by ooee on 10/20/16.
 */
public class Main {
    public static final boolean SHOW_ORIGINAL_LINE = true;

    public static void main(String args[]) {
        if (args.length < 1) {
            System.err.println("Put C file as first parameter");
            System.exit(-1);
        }
        try {
            String preprocess = Preprocess.preprocess(new File(args[0]));
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
            System.err.print("Begin parsing " + args[0] + "...");
            CParser firstPartParser = new CParser(firstPartTokens);
            CParser secondPartParser = new CParser(secondPartTokens);
            System.err.println("Done");
            CParser.CompilationUnitContext firstPartCompilationUnit = firstPartParser.compilationUnit();
            CParser.CompilationUnitContext secondPartCompilationUnit = secondPartParser.compilationUnit();
            TreeToASTVisitor visitor = new TreeToASTVisitor(firstPartCompilationUnit, secondPartTokens);
            System.err.print("Begin AST building...");
            Program program = visitor.visit(secondPartCompilationUnit);
            System.err.println("Done");

            System.err.print("Assigning types to primary expressions...");
            new PrimaryExpressionTypeAssignerVisitor(program);
            System.err.println("Done");

            PDGGenerationVisitor pdgGenerationVisitor = new PDGGenerationVisitor(program);

            //pdgGenerationVisitor.visit(program);
            //FunctionArgumentOrderVisitor functionArgumentOrderVisitor = new FunctionArgumentOrderVisitor();
            //functionArgumentOrderVisitor.visit(program);
            for (Function function : program.getFunction()) {
                System.err.print("Begin PDG generation for function " + function.getIdentifier() + "...");
                PDGNodeCompoundStatement functionBody = pdgGenerationVisitor.visit(function);
                System.err.println("Done");
                System.err.print("Being Useless removal for function " + function.getIdentifier() + "...");
                PDGUselessCodeRemover pdgUselessCodeRemover = new PDGUselessCodeRemover();
                pdgUselessCodeRemover.removeUselessCode(functionBody);
                System.err.println("Done");

                PDGNodeTransitiveReducer pdgNodeTransitiveReducer = new PDGNodeTransitiveReducer();
                pdgNodeTransitiveReducer.reduce(functionBody);
                System.err.print("Being serializing PDG for function " + function.getIdentifier() + "...");
                PDGSorter sorter = new PDGSorterDefault();
                CompoundStatement statement = sorter.sort(functionBody.getBody());
                System.err.println("Done");
                function.setCompoundStatement(statement);
            }
            ProgramIdentifierNormalizerVisitor programIdentifierNormalizerVisitor = new ProgramIdentifierNormalizerVisitor();
            programIdentifierNormalizerVisitor.visit(program);
            System.out.println(program.toCode(SHOW_ORIGINAL_LINE));
        } catch (IOException e) {
            System.err.println(e);
        }
    }
}
