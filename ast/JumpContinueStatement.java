package ast;

import lombok.Value;

/**
 * Created by ooee on 9/25/16.
 */
@Value
public class JumpContinueStatement implements JumpStatement {
    @Override
    public String toCode() {
        return "continue;";
    }
}
