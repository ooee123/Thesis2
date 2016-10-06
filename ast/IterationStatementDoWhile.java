package ast;

import lombok.Value;

/**
 * Created by ooee on 9/25/16.
 */
@Value
public class IterationStatementDoWhile implements IterationStatement {
    private Statement statement;
    private Expression condition;

    @Override
    public String toCode() {
        return String.format("do %s while (%s)", statement.toCode(), condition.toCode());
    }
}
