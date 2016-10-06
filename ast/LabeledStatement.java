package ast;

/**
 * Created by ooee on 9/24/16.
 */

// Has a label for jumps
public class LabeledStatement implements Statement {
    @Override
    public String toCode() {
        return "LABELED STATEMENT";
    }
}
