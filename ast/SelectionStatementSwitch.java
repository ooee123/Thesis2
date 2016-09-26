package ast;

import lombok.Value;

/**
 * Created by ooee on 9/25/16.
 */
@Value
public class SelectionStatementSwitch implements SelectionStatement {
    private Expression expression;
    private Statement statement;
}