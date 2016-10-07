package ast;

import lombok.Value;

/**
 * Created by ooee on 10/6/16.
 */
@Value
public class LabeledIdentifierStatement implements LabeledStatement {
    private String identifier;
    private Statement statement;

    @Override
    public String toCode() {
        return identifier + ": " + statement.toCode();
    }
}
