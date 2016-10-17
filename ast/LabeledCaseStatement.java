package ast;

import lombok.Value;

import java.util.Collections;
import java.util.Set;

/**
 * Created by ooee on 10/6/16.
 */
//TODO: This is incorrect, need to consider for switch statements
@Value
public class LabeledCaseStatement implements LabeledStatement {
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
    public Set<String> getChangedVariables() {
        return conditionalExpression.getChangedVariables();
    }

    @Override
    public boolean isCritical() {
        return true;
    }
}
