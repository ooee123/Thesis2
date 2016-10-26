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
public class LabeledDefaultStatement implements LabeledStatement, CanContainStatements {
    private Statement statement;

    @Override
    public String toCode() {
        return "default: " + statement.toCode();
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
        return true;
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
    public <T> Collection<T> visitEachStatement(Visitor<T, Statement> visitor) {
        return visitor.visit(statement);
    }

    @Override
    public <T> Collection<T> visitAllExpressions(Visitor<T, Expression> visitor) {
        return statement.visitAllExpressions(visitor);
    }

    @Override
    public int pointValue() {
        return statement.pointValue();
    }
}
