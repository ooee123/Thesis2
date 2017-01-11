package ast.statement.impl;

import ast.statement.CanContainStatements;
import ast.expression.ConditionalExpression;
import ast.expression.Expression;
import ast.statement.LabeledStatement;
import ast.statement.Statement;
import com.google.common.collect.Lists;
import lombok.Value;
import visitor.Visitor;

import java.util.*;

/**
 * Created by ooee on 10/6/16.
 */
//TODO: This is incorrect, need to consider for switch statements
@Value
public class LabeledCaseStatement implements LabeledStatement, CanContainStatements {
    private ConditionalExpression conditionalExpression;
    private Statement statement;

    @Override
    public String toCode() {
        return "case " + conditionalExpression.toCode() + ": " + statement.toCode();
    }

    @Override
    public Set<String> getDependantVariables() {
        return conditionalExpression.getDependentVariables();
    }

    @Override
    public boolean isCritical() {
        return true;
    }

    @Override
    public Set<String> getGuaranteedChangedVariables() {
        Set<String> guaranteedChangedVariables = new HashSet<>();
        guaranteedChangedVariables.addAll(conditionalExpression.getGuaranteedChangedVariables());
        guaranteedChangedVariables.addAll(statement.getGuaranteedChangedVariables());
        return guaranteedChangedVariables;
    }

    @Override
    public Set<String> getPotentiallyChangedVariables() {
        Set<String> potentiallyChangedVariables = new HashSet<>();
        potentiallyChangedVariables.addAll(conditionalExpression.getPotentiallyChangedVariables());
        potentiallyChangedVariables.addAll(statement.getPotentiallyChangedVariables());
        return potentiallyChangedVariables;
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
        visitor.visit(conditionalExpression);
    }

    @Override
    public void visitAllExpressions(Visitor<Expression> visitor) {
        visitOwnedExpressions(visitor);
        statement.visitOwnedExpressions(visitor);
    }
}
