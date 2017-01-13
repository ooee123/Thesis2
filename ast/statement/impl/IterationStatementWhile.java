package ast.statement.impl;

import ast.statement.CanContainStatements;
import ast.expression.Expression;
import ast.statement.Statement;
import ast.statement.IterationStatement;
import com.google.common.collect.Lists;
import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NonNull;
import visitor.Visitor;

import java.util.Collection;
import java.util.HashSet;
import java.util.Set;

/**
 * Created by ooee on 9/25/16.
 */
@Data
@AllArgsConstructor
public class IterationStatementWhile implements IterationStatement, CanContainStatements {
    @NonNull private Expression condition;
    @NonNull private Statement statement;
    private String originalString;

    public IterationStatementWhile(Expression condition, Statement statement) {
        this(condition, statement, toCommentTip(condition.toCode()));
    }

    @Override
    public String toCode(boolean showOriginalLine) {
        if (showOriginalLine) {
            return String.format("%s /* %s */ %s", toCommentTip(condition.toCode()), originalString, statement.toCode(showOriginalLine));
        } else {
            return toCode(condition.toCode(), statement.toCode(showOriginalLine));
        }
    }

    @Override
    public Set<String> getDependantVariables() {
        Set<String> dependentVariables = new HashSet<>();
        dependentVariables.addAll(condition.getVariables());
        dependentVariables.addAll(statement.getDependantVariables());
        return dependentVariables;
    }

    @Override
    public Set<String> getPotentiallyChangedVariables() {
        Set<String> potentiallyChangedVariables = new HashSet<>();
        potentiallyChangedVariables.addAll(condition.getPotentiallyChangedVariables());
        potentiallyChangedVariables.addAll(statement.getPotentiallyChangedVariables());
        potentiallyChangedVariables.addAll(statement.getGuaranteedChangedVariables());
        return potentiallyChangedVariables;
    }

    @Override
    public Set<String> getGuaranteedChangedVariables() {
        return condition.getGuaranteedChangedVariables();
    }

    @Override
    public boolean isCritical() {
        return true;
    }

    public static String toCode(String conditionCode, String statementCode) {
        return String.format("%s %s", toCommentTip(conditionCode), statementCode);
    }

    private static String toCommentTip(String condition) {
        return String.format("while (%s)", condition);
    }

    @Override
    public Collection<Statement> getStatementNodes() {
        return Lists.newArrayList(statement);
    }

    @Override
    public void visitEachStatement(Visitor<Statement> visitor) {
        visitor.visit(statement);
    }

    @Override
    public void visitOwnedExpressions(Visitor<Expression> visitor) {
        visitor.visit(condition);
    }

    @Override
    public void visitAllExpressions(Visitor<Expression> visitor) {
        visitOwnedExpressions(visitor);
        statement.visitOwnedExpressions(visitor);
    }
}
