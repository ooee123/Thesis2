package ast;

import com.google.common.collect.Lists;
import lombok.Value;
import pdg.PDGNode;

import java.util.*;
import java.util.stream.Collectors;

/**
 * Created by ooee on 9/24/16.
 */
@Value
public class CompoundStatement implements Statement {
    private List<BlockItem> blockItems;

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

    @Override
    public String toCode() {
        List<String> codes = collectionToCode(blockItems);
        return "{\n" + String.join("\n", codes) + "\n}";
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

    @Override
    public Set<String> getChangedVariables() {
        Set<String> declaredVariables = getDeclaredVariables();
        Set<String> changedVariables = new HashSet<>();
        for (BlockItem blockItem : blockItems) {
            if (blockItem instanceof Statement) {
                changedVariables.addAll(((Statement) blockItem).getChangedVariables());
            }
        }
        changedVariables.removeAll(declaredVariables);
        return changedVariables;
    }

    public Set<String> getDeclaredVariables() {
        Set<String> declaredVariables = new HashSet<>();
        for (BlockItem blockItem : blockItems) {
            if (blockItem instanceof Declaration) {
                List<Declaration.DeclaredVariable> declaredVariablesList = ((Declaration) blockItem).getDeclaredVariables();
                for (Declaration.DeclaredVariable declaredVariable : declaredVariablesList) {
                    declaredVariables.add(declaredVariable.getIdentifier());
                }
            }
        }
        return declaredVariables;
    }

    /*
    public CompoundStatementGroups toCompoundStatementGroups() {
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
}
