package visitor;

import ast.*;
import ast.expression.impl.AssignmentExpressionImpl;
import ast.expression.impl.PrimaryExpressionIdentifier;
import ast.statement.CanContainStatements;
import ast.statement.Statement;
import ast.statement.impl.CompoundStatement;
import ast.statement.impl.CompoundStatementGroups;
import ast.statement.impl.ExpressionStatement;

import java.util.ArrayList;
import java.util.List;

/**
 * Created by ooee on 10/15/16.
 */
public class DeclarationSplitterVisitor {
    public void visit(CompoundStatement statement) {
        List<BlockItem> blockItems = new ArrayList<>();
        CompoundStatementGroups compoundStatementGroups = visitToGroups(statement);
        blockItems.addAll(compoundStatementGroups.getVariableDeclarations());
        blockItems.addAll(compoundStatementGroups.getStatements());
        statement.setBlockItems(blockItems);
    }

    public CompoundStatementGroups visitToGroups(CompoundStatement statement) {
        List<VariableDeclaration> variableDeclarations = new ArrayList<>();
        List<Statement> statements = new ArrayList<>();

        List<BlockItem> blockItems = statement.getBlockItems();
        for (BlockItem blockItem : blockItems) {
            if (blockItem instanceof VariableDeclaration) {
                VariableDeclaration variableDeclaration = (VariableDeclaration) blockItem;
                List<VariableDeclaration.DeclaredVariable> declaredVariables = new ArrayList<>();
                for (VariableDeclaration.DeclaredVariable declaredVariable : variableDeclaration.getDeclaredVariables()) {
                    declaredVariables.add(new VariableDeclaration.DeclaredVariable(declaredVariable.getType(), declaredVariable.getIdentifier()));
                    if (declaredVariable.getInitializer() != null) {
                        String variableName = declaredVariable.getIdentifier().getIdentifier();
                        Statement assignmentStatement = new ExpressionStatement(new AssignmentExpressionImpl(new PrimaryExpressionIdentifier(variableName), AssignmentExpressionImpl.AssignmentOperator.ASSIGNMENT, declaredVariable.getInitializer()));
                        statements.add(assignmentStatement);
                    }
                }
                variableDeclarations.add(new VariableDeclaration(declaredVariables));
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
        return new CompoundStatementGroups(variableDeclarations, statements);
    }
}
