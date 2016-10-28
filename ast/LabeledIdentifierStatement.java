package ast;

import com.google.common.collect.Lists;
import lombok.Value;
import visitor.Visitor;

import java.util.Collection;
import java.util.Set;

/**
 * Created by ooee on 10/6/16.
 */
@Value
public class LabeledIdentifierStatement implements LabeledStatement, CanContainStatements {
    private String identifier;
    private Statement statement;

    @Override
    public String toCode() {
        return identifier + ": " + statement.toCode();
    }

    @Override
    public Set<String> getDependantVariables() {
        return statement.getDependantVariables();
    }
/*
    @Override
    public Set<String> getChangedVariables() {
        return statement.getChangedVariables();
    }
*/
    @Override
    public boolean isCritical() {
        return false;
    }

    @Override
    public Set<String> getGuaranteedChangedVariables() {
        return statement.getGuaranteedChangedVariables();
    }

    @Override
    public Set<String> getPotentiallyChangedVariables() {
        return statement.getPotentiallyChangedVariables();
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
    public void visitAllExpressions(Visitor<Expression> visitor) {
        statement.visitAllExpressions(visitor);
    }
}
