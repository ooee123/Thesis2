package ast;

import java.util.Collection;
import java.util.List;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * Created by ooee on 9/29/16.
 */
public interface BaseElement {
    String toCode(boolean showOriginalLine);

    default List<String> collectionToCode(Collection<? extends BaseElement> baseElements, boolean showOriginalLine) {
        return baseElements.stream().map(elem -> elem.toCode(showOriginalLine)).collect(Collectors.toList());
    }
}
