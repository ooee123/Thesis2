package ast.statement.impl;

import ast.BlockItem;
import ast.statement.CanContainStatements;
import ast.expression.Expression;
import ast.statement.SelectionStatement;
import ast.statement.Statement;
import ast.statement.LabeledStatement;
import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NonNull;
import visitor.Visitor;

import java.util.*;

/**
 * Created by ooee on 9/25/16.
 */
@Data
@AllArgsConstructor
public class SelectionStatementSwitch implements SelectionStatement, CanContainStatements {
    @NonNull private Expression expression;
    @NonNull private CompoundStatement compoundStatement;
    /*
    @NonNull private List<List<BlockItem>> blocks;
    private List<BlockItem> defaultCase;
    */

    private final String originalLine;

    public SelectionStatementSwitch(Expression expression, CompoundStatement compoundStatement) {
        this(expression, compoundStatement, toCommentTip(expression));
    }

    private static String toCommentTip(Expression expression) {
        return String.format("switch (%s)", expression.toCode());
    }

    @Override
    public String toCode(boolean showOriginalLine) {
        String base = toCommentTip(expression);
        if (showOriginalLine) {
            base = base + " /* " + originalLine + " */";
        }
        return String.format("%s %s", base, compoundStatement.toCode(showOriginalLine));
        /*
        StringBuilder builder = new StringBuilder();
        builder.append(String.format("switch (%s) ", expression.toCode()));
        builder.append("{\n");
        for (List<BlockItem> block : blocks) {
            for (BlockItem blockItem : block) {
                builder.append(blockItem.toCode());
            }
        }
        if (defaultCase != null) {
            for (BlockItem blockItem : defaultCase) {
                builder.append(blockItem.toCode());
            }
        }
        builder.append("}\n");
        return builder.toString();
        */
    }

    public boolean hasDefault() {
        for (BlockItem blockItem : compoundStatement.getBlockItems()) {
            if (blockItem instanceof LabeledDefaultStatement) {
                return true;
            }
        }
        return false;
    }

    public List<List<BlockItem>> getCases() {
        int i = 0;
        List<List<BlockItem>> cases = new ArrayList<>();
        List<BlockItem> blockItems = compoundStatement.getBlockItems();
        while (i < blockItems.size()) {
            if (blockItems.get(i) instanceof LabeledCaseStatement) {
                List<BlockItem> caseBlock = new ArrayList<>();
                caseBlock.add(blockItems.get(i++));
                while (i < blockItems.size() && !(blockItems.get(i) instanceof LabeledStatement)) {
                    caseBlock.add(blockItems.get(i++));
                }
                cases.add(caseBlock);
            } else {
                i++;
            }
        }
        return cases;
    }

    public List<BlockItem> getDefaultCase() {
        int i = 0;
        if (hasDefault()) {
            List<BlockItem> blockItems = compoundStatement.getBlockItems();
            while (i < blockItems.size()) {
                if (blockItems.get(i) instanceof LabeledDefaultStatement) {
                    List<BlockItem> defaultCase = new ArrayList<>();
                    defaultCase.add(blockItems.get(i++));
                    while (i < blockItems.size() && !(blockItems.get(i) instanceof LabeledDefaultStatement)) {
                        defaultCase.add(blockItems.get(i++));
                    }
                    return defaultCase;
                } else {
                    i++;
                }
            }
            return null;
        } else {
            return null;
        }
    }

    private List<CompoundStatement> blocksToCompoundStatements() {
        int i = 0;
        List<BlockItem> blockItems = compoundStatement.getBlockItems();
        List<List<BlockItem>> blocks = new ArrayList<>();
        while (i < blockItems.size()) {
            if (blockItems.get(i) instanceof LabeledStatement) {
                List<BlockItem> block = new ArrayList<>();
                block.add(blockItems.get(i));
                i++;
                while (i < blockItems.size() && !(blockItems.get(i) instanceof LabeledStatement)) {
                    block.add(blockItems.get(i));
                    i++;
                }
                blocks.add(block);
            } else {
                i++;
            }
        }
        List<CompoundStatement> compoundStatements = new ArrayList<>();
        for (List<BlockItem> block : blocks) {
            compoundStatements.add(new CompoundStatement(block));
        }
        return compoundStatements;
    }

    @Override
    public Set<String> getDependantVariables() {
        Set<String> dependentVariables = new HashSet<>();
        dependentVariables.addAll(expression.getVariables());
        for (CompoundStatement compoundStatement : blocksToCompoundStatements()) {
            dependentVariables.addAll(compoundStatement.getDependantVariables());
        }
        return dependentVariables;
    }
/*
    @Override
    public Set<String> getChangedVariables() {
        Set<String> changedVariables = new HashSet<>();
        changedVariables.addAll(expression.getGuaranteedChangedVariables());
        changedVariables.addAll(statement.getChangedVariables());
        return changedVariables;
    }
*/
    @Override
    public Set<String> getGuaranteedChangedVariables() {
        if (hasDefault()) {
            List<CompoundStatement> compoundStatements = blocksToCompoundStatements();
            Set<String> intersection = new HashSet<>();
            intersection.addAll(compoundStatements.get(0).getGuaranteedChangedVariables());
            for (CompoundStatement compoundStatement : compoundStatements) {
                intersection.retainAll(compoundStatement.getGuaranteedChangedVariables());
            }
            return intersection;
        } else {
            return new HashSet<>();
        }
    }

    @Override
    public Set<String> getPotentiallyChangedVariables() {
        Set<String> variables = new HashSet<>();
        for (CompoundStatement compoundStatement : blocksToCompoundStatements()) {
            variables.addAll(compoundStatement.getGuaranteedChangedVariables());
            variables.addAll(compoundStatement.getPotentiallyChangedVariables());
        }
        variables.removeAll(getGuaranteedChangedVariables());
        return variables;
    }

    @Override
    public boolean isCritical() {
        return true;
    }

    @Override
    public Collection<Statement> getStatementNodes() {
        Collection<Statement> statements = new ArrayList<>();
        statements.add(compoundStatement);
        return statements;
    }

    @Override
    public void visitEachStatement(Visitor<Statement> visitor) {
        visitor.visit(compoundStatement);
    }

    @Override
    public void visitOwnedExpressions(Visitor<Expression> visitor) {
        visitor.visit(expression);
    }

    @Override
    public void visitAllExpressions(Visitor<Expression> visitor) {
        visitOwnedExpressions(visitor);
        for (CompoundStatement compoundStatement : blocksToCompoundStatements()) {
            compoundStatement.visitAllExpressions(visitor);
        }
    }
}