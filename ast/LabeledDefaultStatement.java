package ast;

import lombok.Value;

/**
 * Created by ooee on 10/6/16.
 */
@Value
public class LabeledDefaultStatement implements LabeledStatement {
    private Statement statement;

    @Override
    public String toCode() {
        return "default: " + statement.toCode();
    }
}
