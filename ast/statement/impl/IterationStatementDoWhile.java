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
public class IterationStatementDoWhile implements IterationStatement, CanContainStatements {
    @NonNull private Statement statement;
    @NonNull private Expression condition;
    private String originalLine;

    public IterationStatementDoWhile(Statement statement, Expression condition) {
       this(statement, condition, toCommentTip(condition));
    }

    private static String toCommentTip(Expression expression) {
        return String.format("while (%s)", expression.toCode());
    }

    @Override
    public String toCode(boolean showOriginalLine) {
        if (showOriginalLine) {
            return String.format("do %s %s; /* %s */", statement.toCode(showOriginalLine), toCommentTip(condition), originalLine);
        } else {
            return String.format("do %s %s;", statement.toCode(showOriginalLine), toCommentTip(condition));
        }
    }

    @Override
    public Set<String> getDependantVariables() {
        Set<String> dependentVariables = new HashSet<>();
        dependentVariables.addAll(condition.getVariables());
        dependentVariables.addAll(statement.getDependantVariables());
        return dependentVariables;
    }
/*
    @Override
    public Set<String> getChangedVariables() {
        Set<String> changedVariables = new HashSet<>();
        changedVariables.addAll(condition.getGuaranteedChangedVariables());
        changedVariables.addAll(statement.getChangedVariables());
        return changedVariables;
    }
*/
    @Override
    public Set<String> getPotentiallyChangedVariables() {
        Set<String> potentiallyChangedVariables = new HashSet<>();
        potentiallyChangedVariables.addAll(statement.getPotentiallyChangedVariables());
        potentiallyChangedVariables.addAll(statement.getPotentiallyChangedVariables());
        return potentiallyChangedVariables;
    }

    @Override
    public Set<String> getGuaranteedChangedVariables() {
        Set<String> guaranteedChangedVariables = new HashSet<>();
        guaranteedChangedVariables.addAll(statement.getGuaranteedChangedVariables());
        guaranteedChangedVariables.addAll(statement.getGuaranteedChangedVariables());
        return guaranteedChangedVariables;
    }

    @Override
    public boolean isCritical() {
        return true;
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
