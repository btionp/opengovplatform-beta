
	DataVizView = {
			init: function() {
				alert('View Loading');
				console.log(this);
				return this;
			},
			load: function(container) {
				
				alert(container);
				console.log('dataviz view load called!');
			},
			_is_exist: function(obj) {
				if(typeof obj != 'undefined') {
					return true;
				}
				return false;
			}
	}.init();
