package visitor;

import ast.*;

import java.util.ArrayList;
import java.util.List;

/**
 * Created by ooee on 10/15/16.
 */
public class DeclarationSplitterVisitor {
    public void visit(CompoundStatement statement) {
        List<BlockItem> blockItems = new ArrayList<>();
        CompoundStatementGroups compoundStatementGroups = visitToGroups(statement);
        blockItems.addAll(compoundStatementGroups.getDeclarations());
        blockItems.addAll(compoundStatementGroups.getStatements());
        statement.setBlockItems(blockItems);
    }

    public CompoundStatementGroups visitToGroups(CompoundStatement statement) {
        List<Declaration> declarations = new ArrayList<>();
        List<Statement> statements = new ArrayList<>();

        List<BlockItem> blockItems = statement.getBlockItems();
        for (BlockItem blockItem : blockItems) {
            if (blockItem instanceof Declaration) {
                Declaration declaration = (Declaration) blockItem;
                List<Declaration.DeclaredVariable> declaredVariables = new ArrayList<>();
                for (Declaration.DeclaredVariable declaredVariable : declaration.getDeclaredVariables()) {
                    declaredVariables.add(new Declaration.DeclaredVariable(declaredVariable.getType(), declaredVariable.getIdentifier()));
                    if (declaredVariable.getInitializer() != null) {
                        String variableName = declaredVariable.getIdentifier();
                        Statement assignmentStatement = new ExpressionStatement(new AssignmentExpressionImpl(new PrimaryExpressionIdentifier(variableName), AssignmentExpressionImpl.AssignmentOperator.ASSIGNMENT, declaredVariable.getInitializer()));
                        statements.add(assignmentStatement);
                    }
                }
                declarations.add(new Declaration(declaredVariables));
            } else if (blockItem instanceof Statement) {
                if (blockItem instanceof CanContainStatements) {
                    for (Statement statement1 : ((CanContainStatements) blockItem).getStatementNodes()) {
                        if (statement1 instanceof CompoundStatement) {
                            visit(((CompoundStatement) statement1));
                        }
                    }
                }
                statements.add(((Statement) blockItem));
            }
        }
        return new CompoundStatementGroups(declarations, statements);
    }
}
