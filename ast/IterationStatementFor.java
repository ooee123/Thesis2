package ast;

import lombok.Value;

/**
 * Created by ooee on 9/25/16.
 */
@Value
public class IterationStatementFor implements IterationStatement {
    private Expression initial;
    private Expression condition;
    private Expression changer;
}
