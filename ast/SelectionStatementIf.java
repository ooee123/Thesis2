package ast;

import lombok.Value;

/**
 * Created by ooee on 9/25/16.
 */
@Value
public class SelectionStatementIf implements SelectionStatement {
    private Expression condition;
    private Statement thenStatement;
    private Statement elseStatement;
}
