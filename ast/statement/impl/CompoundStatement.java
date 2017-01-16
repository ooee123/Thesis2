package ast.statement.impl;

import ast.BlockItem;
import ast.declaration.VariableDeclaration;
import ast.expression.Expression;
import ast.statement.Statement;
import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.Getter;
import lombok.Setter;
import visitor.Visitor;

import java.util.*;

/**
 * Created by ooee on 9/24/16.
 */
@Data
@AllArgsConstructor
public class CompoundStatement implements Statement {
    @Getter @Setter private List<BlockItem> blockItems;

    @Override
    public boolean isCritical() {
        return blockItems.stream().anyMatch(itm -> itm.isCritical());
    }

    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder();
        int number = 1;
        for (BlockItem blockItem : blockItems) {
            builder.append("\n");
            builder.append(number);
            builder.append(": ");
            builder.append(blockItem.toString());
            number++;
        }
        builder.append("\n");
        return builder.toString();
    }

    public static String toCode(String body) {
        return "{\n" + body + "\n}";
    }

    @Override
    public String toCode(boolean showOriginalLine) {
        List<String> codes = collectionToCode(blockItems, showOriginalLine);
        return toCode(String.join("\n", codes));
    }

    public static CompoundStatement addToEnd(BlockItem toBeAdded, Statement body) {
        if (body instanceof CompoundStatement) {
            CompoundStatement compoundStatement = (CompoundStatement) body;
            compoundStatement.getBlockItems().add(toBeAdded);
            return compoundStatement;
        } else {
            List<BlockItem> compoundStatement = new ArrayList<>();
            compoundStatement.add(body);
            compoundStatement.add(toBeAdded);
            return new CompoundStatement(compoundStatement);
        }
    }

    @Override
    public Set<String> getDependantVariables() {
        Set<String> assignedValues = new HashSet<>();
        Set<String> incomingValues = new HashSet<>();
        for (BlockItem blockItem : blockItems) {
            if (blockItem instanceof Statement) {
                Statement statement = (Statement) blockItem;
                Set<String> dependantVariables = statement.getDependantVariables();
                dependantVariables.removeAll(assignedValues);
                incomingValues.addAll(dependantVariables);
                assignedValues.addAll(statement.getChangedVariables());
            }
        }
        return incomingValues;
    }

    public Set<String> getDeclaredVariables() {
        Set<String> declaredVariables = new HashSet<>();
        for (BlockItem blockItem : blockItems) {
            if (blockItem instanceof VariableDeclaration) {
                List<VariableDeclaration.DeclaredVariable> declaredVariablesList = ((VariableDeclaration) blockItem).getDeclaredVariables();
                for (VariableDeclaration.DeclaredVariable declaredVariable : declaredVariablesList) {
                    declaredVariables.add(declaredVariable.getIdentifier().getIdentifier());
                }
            }
        }
        return declaredVariables;
    }

    @Override
    public Set<String> getGuaranteedChangedVariables() {
        Set<String> guaranteedChangedVariables = new HashSet<>();
        for (BlockItem blockItem : blockItems) {
            guaranteedChangedVariables.addAll(blockItem.getGuaranteedChangedVariables());
        }
        guaranteedChangedVariables.removeAll(getDeclaredVariables());
        return guaranteedChangedVariables;
    }

    @Override
    public Set<String> getPotentiallyChangedVariables() {
        Set<String> potentiallyChangedVariables = new HashSet<>();
        for (BlockItem blockItem : blockItems) {
            potentiallyChangedVariables.addAll(blockItem.getPotentiallyChangedVariables());
            potentiallyChangedVariables.removeAll(blockItem.getGuaranteedChangedVariables());
        }
        potentiallyChangedVariables.removeAll(getDeclaredVariables());
        return potentiallyChangedVariables;
    }

    @Override
    public void visitEachStatement(Visitor<Statement> visitor) {
        return;
    }

    @Override
    public void visitOwnedExpressions(Visitor<Expression> visitor) {
        return;
    }
    /*
    public CompoundStatementGroups toCompoundStatementGroups(DeclarationSplitterVisitor declarationSplitter) {
        List<Declaration> declarations = new ArrayList<>();
        List<Statement> statements = new ArrayList<>();

        for (BlockItem blockItem : blockItems) {
            if (blockItem instanceof Declaration) {
                List<Declaration.DeclaredVariable> declaredVariables = ((Declaration) blockItem).getDeclaredVariables();
                for (Declaration.DeclaredVariable declaredVariable : declaredVariables) {
                    if (declaredVariable.getInitializer() != null) {
                        statements.add(new ExpressionStatement(new AssignmentExpressionImpl(new PrimaryExpressionIdentifier(declaredVariable.getIdentifier()))));
                    }
                }
            }
        }

    }
    */

    @Override
    public void visitAllExpressions(Visitor<Expression> visitor) {
        for (BlockItem blockItem : blockItems) {
            blockItem.visitOwnedExpressions(visitor);
        }
    }
}
