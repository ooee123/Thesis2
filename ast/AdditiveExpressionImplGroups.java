package ast;

import lombok.Value;

import java.util.Collection;
import java.util.List;
import java.util.stream.Collectors;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class AdditiveExpressionImplGroups implements AdditiveExpression {
    private Collection<MultiplicativeExpression> addingTerms;
    private Collection<MultiplicativeExpression> subtractingTerms;

    @Override
    public String toCode() {
        List<String> addingTermsCode = addingTerms.stream().map(terms -> terms.toCode()).collect(Collectors.toList());
        String addingTerms = String.join(" + ", addingTermsCode);
        List<String> subtractingTermsCode = subtractingTerms.stream().map(terms -> terms.toCode()).collect(Collectors.toList());
        String subtractingTerms = String.join(" - ", subtractingTermsCode);
        if (!addingTerms.isEmpty() && !subtractingTerms.isEmpty()) {
            return addingTerms + " - " + subtractingTerms;
        } else if (addingTerms.isEmpty()) {
            return "-" + subtractingTerms;
        } else {
            return addingTerms;
        }
    }
}
