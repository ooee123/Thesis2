package ast;

import java.util.Set;

/**
 * Created by ooee on 9/26/16.
 */
public interface CastExpression extends MultiplicativeExpression {
    Set<String> getLValues();
}
