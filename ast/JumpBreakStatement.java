package ast;

import lombok.Value;

/**
 * Created by ooee on 9/25/16.
 */
@Value
public class JumpBreakStatement implements JumpStatement {
    @Override
    public String toCode() {
        return "break;";
    }
}
