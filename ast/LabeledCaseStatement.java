package ast;

import lombok.Value;

import java.util.Collections;
import java.util.Set;

/**
 * Created by ooee on 10/6/16.
 */
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
        return conditionalExpression.getVariables();
    }

    @Override
    public Set<String> getChangedVariables() {
        if (conditionalExpression instanceof Assigning) {
            return ((Assigning) conditionalExpression).getLValues();
        } else {
            return Collections.emptySet();
        }
    }
}
