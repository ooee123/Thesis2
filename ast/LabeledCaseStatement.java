package ast;

import lombok.Value;

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
}
