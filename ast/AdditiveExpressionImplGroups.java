package ast;

import lombok.Value;

import java.util.Collection;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class AdditiveExpressionImplGroups implements AdditiveExpression {
    private Collection<MultiplicativeExpression> addingTerms;
    private Collection<MultiplicativeExpression> subtractingTerms;
}
