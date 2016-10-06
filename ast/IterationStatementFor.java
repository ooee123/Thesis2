package ast;

import lombok.Value;

/**
 * Created by ooee on 9/25/16.
 */
@Value
public class IterationStatementFor implements IterationStatement {
    private Expression initial;
    private Expression condition;
    private Expression iteration;
    private Statement statement;

    @Override
    public String toCode() {
        return String.format("for (%s; %s; %s) %s", initial.toCode(), condition.toCode(), iteration.toCode(), statement.toCode());
    }
}
